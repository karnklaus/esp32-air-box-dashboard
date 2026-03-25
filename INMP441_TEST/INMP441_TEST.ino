#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>

// INMP441 -> ESP32
// VDD -> 3V3
// GND -> GND
// SCK -> GPIO27
// WS  -> GPIO14
// SD  -> GPIO26
// L/R -> GPIO12 (reserved, not used in code)

constexpr i2s_port_t I2S_PORT = I2S_NUM_0;
constexpr int PIN_BCLK = 27;
constexpr int PIN_WS = 14;
constexpr int PIN_SD = 26;
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t SAMPLE_RATE = 16000;
constexpr size_t BUFFER_SAMPLES = 1024;
constexpr size_t PLOT_DOWNSAMPLE = 8;

int32_t samples[BUFFER_SAMPLES];
float envelope = 0.0f;

bool setupI2S() {
  const i2s_config_t config = {
    .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 128,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  const i2s_pin_config_t pins = {
    .bck_io_num = PIN_BCLK,
    .ws_io_num = PIN_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = PIN_SD
  };

  if (i2s_driver_install(I2S_PORT, &config, 0, nullptr) != ESP_OK) {
    return false;
  }

  if (i2s_set_pin(I2S_PORT, &pins) != ESP_OK) {
    return false;
  }

  if (i2s_zero_dma_buffer(I2S_PORT) != ESP_OK) {
    return false;
  }

  return true;
}

bool readI2SBlock(int32_t *buffer, size_t sampleCount, size_t &samplesRead) {
  size_t bytesRead = 0;
  if (i2s_read(I2S_PORT, buffer, sampleCount * sizeof(int32_t), &bytesRead, portMAX_DELAY) != ESP_OK) {
    samplesRead = 0;
    return false;
  }

  samplesRead = bytesRead / sizeof(int32_t);
  return samplesRead > 0;
}

void computeLevels(const int32_t *buffer, size_t count, int32_t &peakOut, int32_t &rmsOut) {
  peakOut = 0;
  double sumSquares = 0.0;

  for (size_t i = 0; i < count; ++i) {
    int32_t sample = buffer[i] >> 8;
    int32_t absSample = abs(sample);

    if (absSample > peakOut) {
      peakOut = absSample;
    }

    sumSquares += static_cast<double>(sample) * static_cast<double>(sample);
  }

  rmsOut = static_cast<int32_t>(sqrt(sumSquares / count));
}

int32_t updateEnvelope(int32_t rawSample) {
  float target = static_cast<float>(abs(rawSample));

  // Fast attack, slow release so speech shape is easier to see.
  if (target > envelope) {
    envelope = envelope * 0.65f + target * 0.35f;
  } else {
    envelope = envelope * 0.92f + target * 0.08f;
  }

  return static_cast<int32_t>(envelope);
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1000);

  Serial.println("INMP441 Serial Plotter Test");
  Serial.println("Open Tools > Serial Plotter");
  Serial.println("Plot columns: raw envelope peak rms");
  Serial.println("Speech view: envelope smoothed for voice shape");

  if (!setupI2S()) {
    Serial.println("I2S init failed");
    while (true) {
      delay(1000);
    }
  }

  Serial.println("I2S init success");
}

void loop() {
  size_t samplesRead = 0;
  if (!readI2SBlock(samples, BUFFER_SAMPLES, samplesRead)) {
    return;
  }

  int32_t peak = 0;
  int32_t rms = 0;
  computeLevels(samples, samplesRead, peak, rms);

  for (size_t i = 0; i < samplesRead; i += PLOT_DOWNSAMPLE) {
    int32_t raw = samples[i] >> 8;
    int32_t env = updateEnvelope(raw);
    Serial.print(raw);
    Serial.print(' ');
    Serial.print(env);
    Serial.print(' ');
    Serial.print(peak);
    Serial.print(' ');
    Serial.println(rms);
  }
}
