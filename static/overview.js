const OVERVIEW_ONLINE_WINDOW_MS = 10000;

const totalEl = document.getElementById("overview-total");
const onlineEl = document.getElementById("overview-online");
const loudEl = document.getElementById("overview-loud");
const avgPm25El = document.getElementById("overview-avg-pm25");
const avgTempEl = document.getElementById("overview-avg-temp");
const avgHumidityEl = document.getElementById("overview-avg-humidity");
const pm25ChartEl = document.getElementById("overview-pm25-chart");
const profileChartEl = document.getElementById("overview-profile-chart");
const tableBodyEl = document.getElementById("overview-table-body");
const overviewMetricTitleEl = document.getElementById("overview-metric-title");
const overviewHistoryTitleEl = document.getElementById("overview-history-title");
const overviewClimateHistoryTitleEl = document.getElementById("overview-climate-history-title");
const districtFilterEl = document.getElementById("filter-district");
const onlineFilterEl = document.getElementById("filter-online");
const loudFilterEl = document.getElementById("filter-loud");
const historyBoxEl = document.getElementById("history-box");
const historyShowPm1El = document.getElementById("history-show-pm1");
const historyShowPm25El = document.getElementById("history-show-pm25");
const historyShowPm10El = document.getElementById("history-show-pm10");
const historyShowTempEl = document.getElementById("history-show-temp");
const historyShowHumidityEl = document.getElementById("history-show-humidity");
const exportCsvBtn = document.getElementById("export-csv-btn");
const historyChartEl = document.getElementById("overview-history-chart");
const climateHistoryChartEl = document.getElementById("overview-climate-history-chart");

let currentBoxes = Array.isArray(window.INITIAL_AIR_BOXES) ? window.INITIAL_AIR_BOXES : [];
let districtOptions = [];

function metricLabel(metric) {
  if (metric === "pm1_0") return "PM1.0";
  if (metric === "pm10") return "PM10";
  if (metric === "temperature_c") return "TEMP";
  if (metric === "humidity_rh") return "HUM";
  return "PM2.5";
}

function selectedHistoryMetrics() {
  const metrics = [];
  if (historyShowPm1El?.checked) metrics.push("pm1_0");
  if (historyShowPm25El?.checked) metrics.push("pm2_5");
  if (historyShowPm10El?.checked) metrics.push("pm10");
  return metrics;
}

function selectedClimateHistoryMetrics() {
  const metrics = [];
  if (historyShowTempEl?.checked) metrics.push("temperature_c");
  if (historyShowHumidityEl?.checked) metrics.push("humidity_rh");
  return metrics;
}

function isOnline(box) {
  if (!box.last_seen) return false;
  const ts = Date.parse(box.last_seen);
  return Number.isFinite(ts) && Date.now() - ts <= OVERVIEW_ONLINE_WINDOW_MS;
}

function compareBoxes(a, b) {
  const aOnlineRank = isOnline(a) ? 0 : 1;
  const bOnlineRank = isOnline(b) ? 0 : 1;
  if (aOnlineRank !== bOnlineRank) {
    return aOnlineRank - bOnlineRank;
  }

  const nameCompare = String(a.name || "NONE").localeCompare(String(b.name || "NONE"), undefined, {
    sensitivity: "base"
  });
  if (nameCompare !== 0) {
    return nameCompare;
  }

  return String(a.id || "").localeCompare(String(b.id || ""), undefined, {
    sensitivity: "base"
  });
}

function seenText(box) {
  if (!box.last_seen) return "Never";
  const ts = Date.parse(box.last_seen);
  if (!Number.isFinite(ts)) return "Never";
  return new Date(ts).toLocaleTimeString([], {
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit"
  });
}

function pmLevelClass(value) {
  const pm = Number(value) || 0;
  if (pm > 40) return "pm-level-red";
  if (pm >= 20) return "pm-level-yellow";
  return "pm-level-green";
}

function mean(values) {
  if (!values.length) return 0;
  return values.reduce((sum, value) => sum + value, 0) / values.length;
}

function climateValue(value) {
  const numeric = Number(value);
  return Number.isFinite(numeric) ? numeric : null;
}

function climateText(value, suffix) {
  const numeric = climateValue(value);
  return numeric === null ? `--.- ${suffix}` : `${numeric.toFixed(1)} ${suffix}`;
}

function chartTimeLabel(timestamp) {
  const ts = Date.parse(timestamp);
  if (!Number.isFinite(ts)) {
    return "--:--:--";
  }
  return new Date(ts).toLocaleTimeString([], {
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit"
  });
}

function updateDistrictOptions(boxes) {
  const nextDistricts = [...new Set(
    boxes
      .map((box) => String(box.district || "").trim())
      .filter(Boolean)
  )].sort((a, b) => a.localeCompare(b));

  if (JSON.stringify(nextDistricts) === JSON.stringify(districtOptions)) {
    return;
  }

  const selected = districtFilterEl.value;
  districtOptions = nextDistricts;
  districtFilterEl.innerHTML = "<option value=\"\">All Districts</option>";
  nextDistricts.forEach((district) => {
    const option = document.createElement("option");
    option.value = district;
    option.textContent = district;
    districtFilterEl.appendChild(option);
  });
  districtFilterEl.value = nextDistricts.includes(selected) ? selected : "";
}

function updateHistoryBoxOptions(boxes) {
  const selected = historyBoxEl.value;
  const options = [...boxes]
    .sort(compareBoxes)
    .map((box) => ({ id: box.id, name: box.name || "NONE" }));

  historyBoxEl.innerHTML = "<option value=\"\">All Units</option>";
  options.forEach((box) => {
    const option = document.createElement("option");
    option.value = box.id;
    option.textContent = `${box.name} (${box.id})`;
    historyBoxEl.appendChild(option);
  });
  historyBoxEl.value = options.some((box) => box.id === selected) ? selected : "";
}

function getFilteredBoxes(boxes) {
  const boxId = historyBoxEl.value.trim();
  const district = districtFilterEl.value.trim();
  const online = onlineFilterEl.value.trim();
  const loud = loudFilterEl.value.trim().toUpperCase();

  return boxes.filter((box) => {
    if (boxId && String(box.id || "").trim() !== boxId) return false;
    if (district && String(box.district || "").trim() !== district) return false;
    if (online === "online" && !isOnline(box)) return false;
    if (online === "offline" && isOnline(box)) return false;
    if (loud === "LOUD" && String(box.sound_state || "").toUpperCase() !== "LOUD") return false;
    if (loud === "QUIET" && String(box.sound_state || "").toUpperCase() !== "QUIET") return false;
    return true;
  });
}

function updateExportUrl() {
  const params = new URLSearchParams();
  if (historyBoxEl.value) params.set("box_id", historyBoxEl.value);
  if (districtFilterEl.value) params.set("district", districtFilterEl.value);
  if (onlineFilterEl.value) params.set("online", onlineFilterEl.value);
  if (loudFilterEl.value) params.set("loud", loudFilterEl.value);
  exportCsvBtn.href = `/api/overview/export.csv?${params.toString()}`;
}

function renderSummary(boxes) {
  const onlineCount = boxes.filter(isOnline).length;
  const loudCount = boxes.filter((box) => String(box.sound_state || "").toUpperCase() === "LOUD").length;
  const avgPm25 = mean(boxes.map((box) => Number(box.pm2_5) || 0));
  const tempValues = boxes.map((box) => climateValue(box.temperature_c)).filter((value) => value !== null);
  const humidityValues = boxes.map((box) => climateValue(box.humidity_rh)).filter((value) => value !== null);

  totalEl.textContent = String(boxes.length);
  onlineEl.textContent = String(onlineCount);
  loudEl.textContent = String(loudCount);
  avgPm25El.textContent = avgPm25.toFixed(1);
  if (avgTempEl) {
    avgTempEl.textContent = tempValues.length ? mean(tempValues).toFixed(1) : "--.-";
  }
  if (avgHumidityEl) {
    avgHumidityEl.textContent = humidityValues.length ? mean(humidityValues).toFixed(1) : "--.-";
  }
}

function renderMetricChart(boxes) {
  const sorted = [...boxes].sort(compareBoxes);
  const maxValue = Math.max(
    1,
    ...sorted.flatMap((box) => [
      Number(box.pm1_0) || 0,
      Number(box.pm2_5) || 0,
      Number(box.pm10) || 0
    ])
  );

  if (overviewMetricTitleEl) {
    overviewMetricTitleEl.textContent = "PM Lines by Unit";
  }

  pm25ChartEl.innerHTML = sorted.map((box) => {
    const pm1 = Number(box.pm1_0) || 0;
    const pm25 = Number(box.pm2_5) || 0;
    const pm10 = Number(box.pm10) || 0;
    const pm1Width = Math.max(4, Math.round((pm1 / maxValue) * 100));
    const pm25Width = Math.max(4, Math.round((pm25 / maxValue) * 100));
    const pm10Width = Math.max(4, Math.round((pm10 / maxValue) * 100));

    return `
      <div class="metric-row metric-row-lines">
        <div class="metric-meta">
          <strong>${box.name || "NONE"}</strong>
          <span>${box.id}</span>
        </div>
        <div class="metric-line-stack">
          <div class="metric-line-track">
            <div class="metric-line-fill metric-line-fill-pm1" style="width:${pm1Width}%"></div>
          </div>
          <div class="metric-line-track">
            <div class="metric-line-fill metric-line-fill-pm25" style="width:${pm25Width}%"></div>
          </div>
          <div class="metric-line-track">
            <div class="metric-line-fill metric-line-fill-pm10" style="width:${pm10Width}%"></div>
          </div>
        </div>
        <div class="metric-value-group">
          <strong class="metric-value metric-value-pm1">${pm1}</strong>
          <strong class="metric-value metric-value-pm25">${pm25}</strong>
          <strong class="metric-value metric-value-pm10">${pm10}</strong>
        </div>
      </div>
    `;
  }).join("");
}

function renderProfileChart(boxes) {
  const avgPm1 = mean(boxes.map((box) => Number(box.pm1_0) || 0));
  const avgPm25 = mean(boxes.map((box) => Number(box.pm2_5) || 0));
  const avgPm10 = mean(boxes.map((box) => Number(box.pm10) || 0));
  const values = [
    { label: "PM1.0", value: avgPm1 },
    { label: "PM2.5", value: avgPm25 },
    { label: "PM10", value: avgPm10 }
  ];
  const maxValue = Math.max(1, ...values.map((item) => item.value));

  profileChartEl.innerHTML = values.map((item) => {
    const height = Math.max(10, Math.round((item.value / maxValue) * 120));
    return `
      <div class="profile-bar-group">
        <div class="profile-bar ${pmLevelClass(item.value)}" style="height:${height}px"></div>
        <strong>${item.value.toFixed(1)}</strong>
        <span>${item.label}</span>
      </div>
    `;
  }).join("");
}

function renderTable(boxes) {
  const sorted = [...boxes].sort(compareBoxes);
  tableBodyEl.innerHTML = sorted.map((box) => `
    <tr>
      <td>${box.name || "NONE"}</td>
      <td>${box.id}</td>
      <td><span class="table-chip ${pmLevelClass(box.pm2_5)}">${Number(box.pm2_5) || 0}</span></td>
      <td>${climateText(box.temperature_c, "C")}</td>
      <td>${climateText(box.humidity_rh, "%RH")}</td>
      <td>${box.sound_state || "QUIET"}</td>
      <td>${isOnline(box) ? "ONLINE" : "OFFLINE"}</td>
      <td>${seenText(box)}</td>
    </tr>
  `).join("");
}

function renderTrendChart(targetEl, titleEl, history, metrics, metricTheme, emptyMessage) {
  if (!targetEl) {
    return;
  }

  if (!history.length || !metrics.length) {
    targetEl.innerHTML = `
      <text x="50%" y="50%" text-anchor="middle" class="history-empty">
        ${history.length ? "Select at least one metric" : emptyMessage}
      </text>
    `;
    return;
  }

  if (titleEl) {
    titleEl.textContent = `${metrics.map(metricLabel).join(" / ")} Trend`;
  }

  const width = 800;
  const height = 220;
  const paddingLeft = 44;
  const paddingRight = 22;
  const paddingTop = 20;
  const paddingBottom = 34;
  const values = history.flatMap((entry) => metrics.map((metric) => Number(entry[metric]) || 0));
  const rawMax = Math.max(1, ...values);
  const rawMin = Math.min(...values);
  const span = Math.max(4, rawMax - rawMin);
  const minValue = Math.max(0, Math.floor(rawMin - span * 0.15));
  const maxValue = Math.ceil(rawMax + span * 0.15);
  const chartBottom = height - paddingBottom;
  const chartRight = width - paddingRight;
  const usableWidth = chartRight - paddingLeft;
  const usableHeight = height - paddingTop - paddingBottom;
  const midValue = Math.round((minValue + maxValue) / 2);

  const pointSet = history.map((entry, index) => {
    const x = history.length === 1
      ? width / 2
      : paddingLeft + (index / (history.length - 1)) * (chartRight - paddingLeft);
    const valuesByMetric = Object.fromEntries(metrics.map((metric) => [
      metric,
      Number(entry[metric]) || 0
    ]));
    return { x, valuesByMetric, timestamp: entry.timestamp };
  });

  const first = pointSet[0];
  const last = pointSet[pointSet.length - 1];
  const seriesSvg = metrics.map((metric) => {
    const themed = metricTheme[metric];
    const metricPoints = pointSet.map((point) => {
      const value = point.valuesByMetric[metric];
      const y = paddingTop + usableHeight - ((value - minValue) / Math.max(1, maxValue - minValue)) * usableHeight;
      return { x: point.x, y, value };
    });
    const polyline = metricPoints.map((point) => `${point.x},${point.y}`).join(" ");
    const areaPath = `M ${metricPoints[0].x} ${chartBottom} L ${metricPoints.map((point) => `${point.x} ${point.y}`).join(" L ")} L ${metricPoints[metricPoints.length - 1].x} ${chartBottom} Z`;
    const lastPoint = metricPoints[metricPoints.length - 1];

    return `
      <path d="${areaPath}" class="history-area ${themed.area}"></path>
      <polyline points="${polyline}" class="history-line ${themed.line}"></polyline>
      ${metricPoints.map((point) => `
        <circle cx="${point.x}" cy="${point.y}" r="3.5" class="history-dot ${themed.dot}"></circle>
      `).join("")}
      <text x="${lastPoint.x}" y="${Math.max(14, lastPoint.y - 10)}" text-anchor="end" class="history-label ${themed.line}">
        ${lastPoint.value}
      </text>
    `;
  }).join("");

  targetEl.innerHTML = `
    <line x1="${paddingLeft}" y1="${chartBottom}" x2="${chartRight}" y2="${chartBottom}" class="history-axis"></line>
    <line x1="${paddingLeft}" y1="${paddingTop}" x2="${paddingLeft}" y2="${chartBottom}" class="history-axis"></line>
    <line x1="${paddingLeft}" y1="${paddingTop}" x2="${chartRight}" y2="${paddingTop}" class="history-grid"></line>
    <line x1="${paddingLeft}" y1="${paddingTop + usableHeight / 2}" x2="${chartRight}" y2="${paddingTop + usableHeight / 2}" class="history-grid"></line>
    <line x1="${paddingLeft}" y1="${chartBottom}" x2="${chartRight}" y2="${chartBottom}" class="history-grid"></line>
    <text x="${paddingLeft - 8}" y="${paddingTop + 4}" text-anchor="end" class="history-tick">${maxValue}</text>
    <text x="${paddingLeft - 8}" y="${paddingTop + usableHeight / 2 + 4}" text-anchor="end" class="history-tick">${midValue}</text>
    <text x="${paddingLeft - 8}" y="${chartBottom + 4}" text-anchor="end" class="history-tick">${minValue}</text>
    ${seriesSvg}
    <text x="${first.x}" y="${chartBottom + 20}" text-anchor="start" class="history-footnote">${chartTimeLabel(first.timestamp)}</text>
    <text x="${last.x}" y="${chartBottom + 20}" text-anchor="end" class="history-footnote">${chartTimeLabel(last.timestamp)}</text>
  `;
}

function renderHistoryChart(history, metrics) {
  renderTrendChart(
    historyChartEl,
    overviewHistoryTitleEl,
    history,
    metrics,
    {
      pm1_0: { line: "history-line-pm1", dot: "history-dot-pm1", area: "history-area-pm1" },
      pm2_5: { line: "history-line-pm25", dot: "history-dot-pm25", area: "history-area-pm25" },
      pm10: { line: "history-line-pm10", dot: "history-dot-pm10", area: "history-area-pm10" }
    },
    "No history available"
  );
}

function renderClimateHistoryChart(history, metrics) {
  const climateHistory = history.filter((entry) =>
    climateValue(entry.temperature_c) !== null || climateValue(entry.humidity_rh) !== null
  );

  renderTrendChart(
    climateHistoryChartEl,
    overviewClimateHistoryTitleEl,
    climateHistory,
    metrics,
    {
      temperature_c: { line: "history-line-temp", dot: "history-dot-temp", area: "history-area-temp" },
      humidity_rh: { line: "history-line-humidity", dot: "history-dot-humidity", area: "history-area-humidity" }
    },
    "No climate history available"
  );
}

async function refreshHistory() {
  try {
    const params = new URLSearchParams();
    if (historyBoxEl.value) params.set("box_id", historyBoxEl.value);
    params.set("limit", "48");
    const response = await fetch(`/api/overview/history?${params.toString()}`, {
      headers: { Accept: "application/json" },
      cache: "no-store"
    });
    if (!response.ok) return;
    const history = await response.json();
    if (!Array.isArray(history)) return;
    renderHistoryChart(history, selectedHistoryMetrics());
    renderClimateHistoryChart(history, selectedClimateHistoryMetrics());
  } catch (error) {
    console.debug("history refresh failed", error);
  }
}

function renderOverview(boxes) {
  updateDistrictOptions(boxes);
  updateHistoryBoxOptions(boxes);
  updateExportUrl();

  const filtered = getFilteredBoxes(boxes);
  renderSummary(filtered);
  renderMetricChart(filtered);
  renderProfileChart(filtered);
  renderTable(filtered);
}

async function refreshOverview() {
  try {
    const response = await fetch("/api/air-boxes", {
      headers: { Accept: "application/json" },
      cache: "no-store"
    });
    if (!response.ok) return;
    const boxes = await response.json();
    if (!Array.isArray(boxes)) return;
    currentBoxes = boxes;
    renderOverview(currentBoxes);
    await refreshHistory();
  } catch (error) {
    console.debug("overview refresh failed", error);
  }
}

[districtFilterEl, onlineFilterEl, loudFilterEl].forEach((control) => {
  control.addEventListener("change", () => {
    renderOverview(currentBoxes);
    refreshHistory();
  });
});

historyBoxEl.addEventListener("change", refreshHistory);
historyBoxEl.addEventListener("change", () => {
  renderOverview(currentBoxes);
});
[historyShowPm1El, historyShowPm25El, historyShowPm10El, historyShowTempEl, historyShowHumidityEl].forEach((control) => {
  control?.addEventListener("change", refreshHistory);
});

renderOverview(currentBoxes);
refreshHistory();
window.setInterval(refreshOverview, 1000);
