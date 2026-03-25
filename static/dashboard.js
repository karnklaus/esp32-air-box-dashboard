const bangkokCenter = [13.7563, 100.5018];
const ONLINE_WINDOW_MS = 10000;

const map = L.map("map", {
  zoomControl: true,
  minZoom: 10
}).setView(bangkokCenter, 11);

L.tileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", {
  attribution: "&copy; OpenStreetMap contributors"
}).addTo(map);

const markers = new Map();
const cards = new Map();
const airBoxes = new Map(window.AIR_BOXES.map((box) => [box.id, box]));
const dragPinIcon = L.divIcon({
  className: "drag-pin-wrap",
  html: '<div class="drag-pin"></div>',
  iconSize: [20, 20],
  iconAnchor: [10, 10]
});

let activeBoxId = window.AIR_BOXES[0]?.id ?? null;
let editingBoxId = null;
let editHandle = null;
let pinPickerEnabled = false;
let autosaveTimer = null;
let deleteModalResolver = null;

const form = document.getElementById("box-form");
const formStatus = document.getElementById("form-status");
const inlineEditor = document.getElementById("inline-editor");
const boxList = document.getElementById("box-list");
const pinToggleButton = document.getElementById("pin-toggle-btn");
const totalBoxesStat = document.querySelector(".stat-card strong");
const deleteModal = document.getElementById("delete-modal");
const deleteModalText = document.getElementById("delete-modal-text");
const deleteCancelButton = document.getElementById("delete-cancel-btn");
const deleteConfirmButton = document.getElementById("delete-confirm-btn");
const editorDeleteButton = document.getElementById("editor-delete-btn");
const fields = {
  id: document.getElementById("box-id"),
  name: document.getElementById("box-name"),
  district: document.getElementById("box-district"),
  lat: document.getElementById("box-lat"),
  lng: document.getElementById("box-lng")
};

function closeDeleteModal(confirmed = false) {
  if (!deleteModal) return;

  deleteModal.classList.add("hidden");
  deleteModal.setAttribute("aria-hidden", "true");

  if (deleteModalResolver) {
    deleteModalResolver(confirmed);
    deleteModalResolver = null;
  }
}

function openDeleteModal(boxId) {
  if (!deleteModal || !deleteModalText) {
    return Promise.resolve(false);
  }

  const box = airBoxes.get(boxId);
  const status = box ? statusMeta(box) : { label: "OFFLINE", seenText: "Never" };
  const nameEl = deleteModalText.querySelector(".delete-box-line-name");
  const statusEl = deleteModalText.querySelector(".delete-box-line-status");
  const metaEl = deleteModalText.querySelector(".delete-box-meta");
  const coordsEl = deleteModalText.querySelector(".delete-box-coords");
  const seenEl = deleteModalText.querySelector(".delete-box-seen");

  if (nameEl) {
    nameEl.textContent = `Name : ${box?.name ?? boxId}`;
  }
  if (statusEl) {
    statusEl.textContent = `Status : ${status.label}`;
  }
  if (metaEl) {
    metaEl.textContent = `${boxId} · ${box?.district ?? ""}`.trim();
  }
  if (coordsEl) {
    coordsEl.textContent = box ? `${box.lat}, ${box.lng}` : "";
  }
  if (seenEl) {
    seenEl.textContent = `Last seen ${status.seenText}`;
  }

  deleteModal.classList.remove("hidden");
  deleteModal.setAttribute("aria-hidden", "false");

  return new Promise((resolve) => {
    deleteModalResolver = resolve;
  });
}

function pmLevelClass(value) {
  const pm = Number(value) || 0;
  if (pm > 40) return "pm-level-red";
  if (pm >= 20) return "pm-level-yellow";
  return "pm-level-green";
}

function statusMeta(box) {
  const seenAt = box.last_seen ? Date.parse(box.last_seen) : NaN;
  const online = Number.isFinite(seenAt) && (Date.now() - seenAt) <= ONLINE_WINDOW_MS;
  return {
    online,
    label: online ? "ONLINE" : "OFFLINE",
    seenText: Number.isFinite(seenAt)
      ? new Date(seenAt).toLocaleTimeString([], {
          hour: "2-digit",
          minute: "2-digit",
          second: "2-digit"
        })
      : "Never"
  };
}

function compareBoxes(a, b) {
  const aOnlineRank = statusMeta(a).online ? 0 : 1;
  const bOnlineRank = statusMeta(b).online ? 0 : 1;
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

function sortedAirBoxes() {
  return Array.from(airBoxes.values()).sort(compareBoxes);
}

function popupHtml(box) {
  const status = statusMeta(box);
  return `
    <div class="map-popup">
      <p class="popup-title">${box.name}</p>
      <p class="popup-meta">${box.id} · ${box.district}</p>
      <div class="popup-status-row">
        <span class="pill ${box.sound_state.toLowerCase()}">${box.sound_state}</span>
        <span class="pill ${status.online ? "online" : "offline"}">${status.label}</span>
      </div>
      <div class="popup-pm-strip">
        <div class="popup-pm-chip ${pmLevelClass(box.pm1_0)}">
          <span class="popup-pm-label">PM1.0</span>
          <strong class="popup-pm-value">${box.pm1_0}</strong>
        </div>
        <div class="popup-pm-chip popup-pm-chip-focus ${pmLevelClass(box.pm2_5)}">
          <span class="popup-pm-label">PM2.5</span>
          <strong class="popup-pm-value">${box.pm2_5}</strong>
        </div>
        <div class="popup-pm-chip ${pmLevelClass(box.pm10)}">
          <span class="popup-pm-label">PM10</span>
          <strong class="popup-pm-value">${box.pm10}</strong>
        </div>
      </div>
      <p class="popup-seen">Last seen ${status.seenText}</p>
    </div>
  `;
}

function setPinToggleState(enabled) {
  pinPickerEnabled = enabled;
  pinToggleButton.classList.toggle("active", enabled);
  pinToggleButton.setAttribute("aria-pressed", String(enabled));
  pinToggleButton.innerHTML = enabled ? "&#128205; ON" : "&#128205;";
  pinToggleButton.title = enabled ? "Disable map pin picker" : "Pick location on map";
}

function updateForm(box) {
  fields.id.value = box.id;
  fields.name.value = box.name ?? "NONE";
  fields.district.value = box.district ?? "";
  fields.lat.value = box.lat;
  fields.lng.value = box.lng;
}

function syncEditorFieldsFromBox(box) {
  if (!box) return;

  if (document.activeElement !== fields.name) {
    fields.name.value = box.name ?? "NONE";
  }
  if (document.activeElement !== fields.district) {
    fields.district.value = box.district ?? "";
  }
  if (document.activeElement !== fields.lat) {
    fields.lat.value = box.lat;
  }
  if (document.activeElement !== fields.lng) {
    fields.lng.value = box.lng;
  }
}

function syncLatLngFields(latlng) {
  fields.lat.value = Number(latlng.lat).toFixed(6);
  fields.lng.value = Number(latlng.lng).toFixed(6);
}

function removeEditHandle() {
  if (editHandle) {
    map.removeLayer(editHandle);
    editHandle = null;
  }
  map.closePopup();
}

function ensureEditHandle(box) {
  const latlng = [box.lat, box.lng];

  if (!editHandle) {
    editHandle = L.marker(latlng, {
      draggable: false,
      icon: dragPinIcon,
      zIndexOffset: 1000
    });
  } else {
    editHandle.setLatLng(latlng);
  }

  if (!map.hasLayer(editHandle)) {
    editHandle.addTo(map);
  }

  map.flyTo(latlng, Math.max(map.getZoom(), 13), { duration: 0.45 });
}

function closeInlineEditor() {
  editingBoxId = null;
  inlineEditor.classList.add("hidden");
  inlineEditor.dataset.boxId = "";
  formStatus.textContent = "";
  boxList.after(inlineEditor);
  setPinToggleState(false);
  removeEditHandle();

  cards.forEach((card) => {
    card.classList.remove("editing");
  });
}

function openInlineEditor(boxId, forceOpen = false) {
  const box = airBoxes.get(boxId);
  const card = cards.get(boxId);
  if (!box || !card) return;

  if (!forceOpen && editingBoxId === boxId && !inlineEditor.classList.contains("hidden")) {
    closeInlineEditor();
    return;
  }

  editingBoxId = boxId;
  updateForm(box);
  inlineEditor.dataset.boxId = boxId;
  formStatus.textContent = "";

  cards.forEach((candidate) => {
    candidate.classList.toggle("editing", candidate === card);
  });

  card.after(inlineEditor);
  inlineEditor.classList.remove("hidden");
  setPinToggleState(false);
  removeEditHandle();
}

function bindEditButton(container, boxId) {
  const editButton = container.querySelector(".edit-btn");
  if (!editButton) return;

  editButton.addEventListener("click", (event) => {
    event.stopPropagation();
    setActiveCard(boxId);
    openInlineEditor(boxId);
  });
}

function bindDeleteButton(container, boxId) {
  const deleteButton = container.querySelector(".delete-btn");
  if (!deleteButton) return;

  deleteButton.addEventListener("click", async (event) => {
    event.stopPropagation();
    const confirmed = await openDeleteModal(boxId);
    if (!confirmed) return;

    try {
      const response = await fetch(`/api/air-boxes/${boxId}`, { method: "DELETE" });
      const result = await response.json();
      if (!response.ok) {
        throw new Error(result.error || "Delete failed");
      }

      const marker = markers.get(boxId);
      if (marker) {
        map.removeLayer(marker);
        markers.delete(boxId);
      }

      const card = cards.get(boxId);
      if (card) {
        card.remove();
        cards.delete(boxId);
      }

      airBoxes.delete(boxId);

      if (editingBoxId === boxId) {
        closeInlineEditor();
      }

      if (activeBoxId === boxId) {
        activeBoxId = airBoxes.size > 0 ? sortedAirBoxes()[0]?.id ?? null : null;
        if (activeBoxId) {
          setActiveCard(activeBoxId);
        }
      }

      if (totalBoxesStat) {
        totalBoxesStat.textContent = String(airBoxes.size);
      }
    } catch (error) {
      formStatus.textContent = error.message;
    }
  });
}

function cardMarkup(box) {
  const status = statusMeta(box);
  return `
    <div class="box-head">
      <div class="box-title-row">
        <span class="box-marker-cue" aria-hidden="true"></span>
        <strong>${box.name}</strong>
      </div>
      <div class="box-actions">
        <span class="pill ${box.sound_state.toLowerCase()}">${box.sound_state}</span>
        <span class="pill ${status.online ? "online" : "offline"}">${status.label}</span>
        <button type="button" class="gear-btn edit-btn" data-edit-id="${box.id}" aria-label="Edit box">&#9881;</button>
      </div>
    </div>
    <div class="box-meta-row">
      <p>${box.id} · ${box.district}</p>
    </div>
    <div class="pm-strip">
      <div class="pm-chip ${pmLevelClass(box.pm1_0)}">
        <span class="pm-chip-label">PM1.0</span>
        <strong class="pm-chip-value">${box.pm1_0}</strong>
      </div>
      <div class="pm-chip pm-chip-focus ${pmLevelClass(box.pm2_5)}">
        <span class="pm-chip-label">PM2.5</span>
        <strong class="pm-chip-value">${box.pm2_5}</strong>
      </div>
      <div class="pm-chip ${pmLevelClass(box.pm10)}">
        <span class="pm-chip-label">PM10</span>
        <strong class="pm-chip-value">${box.pm10}</strong>
      </div>
    </div>
    <p>${box.lat}, ${box.lng}</p>
    <p class="box-last-seen">Last seen ${status.seenText}</p>
  `;
}

function applyCardStatusClass(card, box) {
  const online = statusMeta(box).online;
  card.classList.toggle("status-online", online);
  card.classList.toggle("status-offline", !online);
}

function attachCardInteractions(card, box) {
  card.addEventListener("click", () => {
    const currentBox = airBoxes.get(box.id);
    const marker = markers.get(box.id);
    if (!currentBox) return;
    map.flyTo([currentBox.lat, currentBox.lng], 13, { duration: 0.7 });
    marker?.openPopup();
    setActiveCard(box.id);
  });
  bindEditButton(card, box.id);
  bindDeleteButton(card, box.id);
}

function createCard(box) {
  const card = document.createElement("article");
  card.className = "box-card";
  card.dataset.boxId = box.id;
  card.innerHTML = cardMarkup(box);
  applyCardStatusClass(card, box);
  cards.set(box.id, card);
  attachCardInteractions(card, box);
  boxList.append(card);
}

function reorderCardList(force = false, preserveEditor = false) {
  if (!force && editingBoxId && !inlineEditor.classList.contains("hidden")) {
    return;
  }

  sortedAirBoxes().forEach((box) => {
    const card = cards.get(box.id);
    if (card) {
      boxList.appendChild(card);
    }
  });

  if (preserveEditor && editingBoxId && !inlineEditor.classList.contains("hidden")) {
    const activeCard = cards.get(editingBoxId);
    if (activeCard) {
      activeCard.after(inlineEditor);
    }
  }
}

function updateCard(box) {
  const card = cards.get(box.id);
  if (!card) return;

  card.innerHTML = cardMarkup(box);
  applyCardStatusClass(card, box);
  bindEditButton(card, box.id);
  bindDeleteButton(card, box.id);
}

function updateMarker(box) {
  const marker = markers.get(box.id);
  if (!marker) return;
  const online = statusMeta(box).online;

  marker.setLatLng([box.lat, box.lng]);
  marker.setStyle({
    fillColor: online ? "#1d7a41" : "#d61f35",
    color: "#ffffff"
  });
  marker.bindPopup(popupHtml(box));
}

function previewMarkerPosition(boxId, latlng) {
  const box = airBoxes.get(boxId);
  const marker = markers.get(boxId);
  if (!box || !marker) return;

  box.lat = Number(latlng.lat).toFixed(6);
  box.lng = Number(latlng.lng).toFixed(6);
  marker.setLatLng([Number(box.lat), Number(box.lng)]);
  marker.bindPopup(popupHtml(box));
}

function setActiveCard(id) {
  activeBoxId = id;
  cards.forEach((card, cardId) => {
    card.classList.toggle("active", cardId === id);
  });

  const box = airBoxes.get(id);
  if (!box) return;

  if (editingBoxId === id && !inlineEditor.classList.contains("hidden")) {
    syncEditorFieldsFromBox(box);
    return;
  }

  updateForm(box);
}

async function persistLocation(boxId, latlng) {
  const box = airBoxes.get(boxId);
  if (!box) return;

  const payload = {
    lat: Number(latlng.lat.toFixed(6)),
    lng: Number(latlng.lng.toFixed(6))
  };

  const response = await fetch(`/api/air-boxes/${boxId}`, {
    method: "PATCH",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(payload)
  });

  const result = await response.json();
  if (!response.ok) {
    throw new Error(result.error || "Location save failed");
  }

  airBoxes.set(result.id, { ...box, ...result });
  updateCard(airBoxes.get(result.id));
  updateMarker(airBoxes.get(result.id));

  if (activeBoxId === result.id) {
    setActiveCard(result.id);
  }
}

async function saveEditorChanges() {
  if (!editingBoxId) return;

  const currentBox = airBoxes.get(editingBoxId);
  const payload = {
    name: fields.name.value.trim() || "NONE",
    district: fields.district.value.trim(),
    lat: Number(fields.lat.value),
    lng: Number(fields.lng.value)
  };

  if (Number.isNaN(payload.lat) || Number.isNaN(payload.lng)) {
    formStatus.textContent = "Invalid latitude/longitude";
    return;
  }

  try {
    const response = await fetch(`/api/air-boxes/${editingBoxId}`, {
      method: "PATCH",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload)
    });

    const result = await response.json();
    if (!response.ok) {
      throw new Error(result.error || "Save failed");
    }

    airBoxes.set(result.id, { ...airBoxes.get(result.id), ...result });
    updateCard(airBoxes.get(result.id));
    updateMarker(airBoxes.get(result.id));

    if (String(currentBox?.name || "NONE") !== String(result.name || "NONE")) {
      reorderCardList(true, true);
    }

    if (activeBoxId === result.id) {
      setActiveCard(result.id);
    }
    if (editingBoxId === result.id) {
      syncEditorFieldsFromBox(airBoxes.get(result.id));
    }

    formStatus.textContent = "";
  } catch (error) {
    formStatus.textContent = error.message;
  }
}

function queueEditorAutosave() {
  if (autosaveTimer) {
    window.clearTimeout(autosaveTimer);
  }
  autosaveTimer = window.setTimeout(() => {
    saveEditorChanges();
  }, 350);
}

async function refreshAirBoxes() {
  try {
    const response = await fetch("/api/air-boxes", {
      headers: { Accept: "application/json" },
      cache: "no-store"
    });
    if (!response.ok) return;

    const latestBoxes = await response.json();
    if (!Array.isArray(latestBoxes)) return;

    let shouldReorder = false;

    latestBoxes.forEach((box) => {
      const existing = airBoxes.get(box.id);
      if (!existing) {
        airBoxes.set(box.id, box);
        const marker = L.circleMarker([box.lat, box.lng], {
          radius: 10,
          color: "#ffffff",
          weight: 2,
          fillColor: statusMeta(box).online ? "#1d7a41" : "#d61f35",
          fillOpacity: 0.92
        })
          .addTo(map)
          .bindPopup(popupHtml(box));

        marker.on("click", () => {
          setActiveCard(box.id);
        });
        markers.set(box.id, marker);
        createCard(box);
        shouldReorder = true;
        return;
      }

      if (String(existing.name || "NONE") !== String(box.name || "NONE")) {
        shouldReorder = true;
      }

      airBoxes.set(box.id, { ...existing, ...box });
      updateCard(airBoxes.get(box.id));
      updateMarker(airBoxes.get(box.id));

      if (activeBoxId === box.id && editingBoxId !== box.id) {
        setActiveCard(box.id);
      }

      if (editingBoxId === box.id) {
        syncEditorFieldsFromBox(airBoxes.get(box.id));
      }
    });

    if (shouldReorder) {
      reorderCardList(true);
    }

    if (totalBoxesStat) {
      totalBoxesStat.textContent = String(latestBoxes.length);
    }
  } catch (error) {
    console.debug("refresh failed", error);
  }
}

function showConfirmPopup(latlng) {
  if (!editingBoxId) return;

  if (editHandle) {
    editHandle.setLatLng(latlng);
  }

  const content = document.createElement("div");
  content.className = "confirm-popup-body";

  const title = document.createElement("p");
  title.className = "confirm-popup-title";
  title.textContent = "Save this location?";

  const actions = document.createElement("div");
  actions.className = "confirm-popup-actions";

  const saveButton = document.createElement("button");
  saveButton.type = "button";
  saveButton.className = "confirm-popup-btn confirm-popup-save";
  saveButton.textContent = "Save";

  const cancelButton = document.createElement("button");
  cancelButton.type = "button";
  cancelButton.className = "confirm-popup-btn confirm-popup-cancel";
  cancelButton.textContent = "Cancel";

  actions.append(saveButton, cancelButton);
  content.append(title, actions);
  L.DomEvent.disableClickPropagation(content);

  L.popup({
    closeButton: false,
    autoClose: true,
    className: "confirm-popup"
  })
    .setLatLng(latlng)
    .setContent(content)
    .openOn(map);

  saveButton.addEventListener("click", async () => {
    syncLatLngFields(latlng);
    previewMarkerPosition(editingBoxId, latlng);
    try {
      await persistLocation(editingBoxId, latlng);
      setPinToggleState(false);
      if (editHandle) {
        map.removeLayer(editHandle);
        editHandle = null;
      }
      map.closePopup();
    } catch (error) {
      formStatus.textContent = error.message;
    }
  }, { once: true });

  cancelButton.addEventListener("click", () => {
    map.closePopup();
  }, { once: true });
}

map.on("click", (event) => {
  if (!editingBoxId || !pinPickerEnabled) return;
  showConfirmPopup(event.latlng);
});

pinToggleButton.addEventListener("click", () => {
  if (!editingBoxId) return;

  const box = airBoxes.get(editingBoxId);
  if (!box) return;

  if (pinPickerEnabled) {
    setPinToggleState(false);
    removeEditHandle();
    return;
  }

  setPinToggleState(true);
  ensureEditHandle({
    lat: Number(fields.lat.value) || box.lat,
    lng: Number(fields.lng.value) || box.lng
  });
});

window.AIR_BOXES.forEach((box) => {
  const marker = L.circleMarker([box.lat, box.lng], {
    radius: 10,
    color: "#ffffff",
    weight: 2,
    fillColor: statusMeta(box).online ? "#1d7a41" : "#d61f35",
    fillOpacity: 0.92
  })
    .addTo(map)
    .bindPopup(popupHtml(box));

  marker.on("click", () => {
    setActiveCard(box.id);
  });
  markers.set(box.id, marker);

  const card = document.querySelector(`[data-box-id="${box.id}"]`);
  if (card) {
    cards.set(box.id, card);
    attachCardInteractions(card, box);
  }
});

reorderCardList(true);
activeBoxId = sortedAirBoxes()[0]?.id ?? null;

if (activeBoxId) {
  setActiveCard(activeBoxId);
}

form.addEventListener("submit", async (event) => {
  event.preventDefault();
  await saveEditorChanges();
});

[fields.name, fields.district, fields.lat, fields.lng].forEach((field) => {
  field.addEventListener("change", queueEditorAutosave);
  field.addEventListener("blur", queueEditorAutosave);
});

if (deleteCancelButton) {
  deleteCancelButton.addEventListener("click", () => closeDeleteModal(false));
}

if (deleteConfirmButton) {
  deleteConfirmButton.addEventListener("click", () => closeDeleteModal(true));
}

if (deleteModal) {
  deleteModal.addEventListener("click", (event) => {
    if (event.target === deleteModal || event.target.hasAttribute("data-delete-close")) {
      closeDeleteModal(false);
    }
  });
}

if (editorDeleteButton) {
  editorDeleteButton.addEventListener("click", async () => {
    const boxId = inlineEditor.dataset.boxId;
    if (!boxId) return;

    const confirmed = await openDeleteModal(boxId);
    if (!confirmed) return;

    try {
      const response = await fetch(`/api/air-boxes/${boxId}`, { method: "DELETE" });
      const result = await response.json();
      if (!response.ok) {
        throw new Error(result.error || "Delete failed");
      }

      const marker = markers.get(boxId);
      if (marker) {
        map.removeLayer(marker);
        markers.delete(boxId);
      }

      const card = cards.get(boxId);
      if (card) {
        card.remove();
        cards.delete(boxId);
      }

      airBoxes.delete(boxId);
      closeInlineEditor();

      if (activeBoxId === boxId) {
        activeBoxId = airBoxes.size > 0 ? sortedAirBoxes()[0]?.id ?? null : null;
        if (activeBoxId) {
          setActiveCard(activeBoxId);
        }
      }

      if (totalBoxesStat) {
        totalBoxesStat.textContent = String(airBoxes.size);
      }
    } catch (error) {
      formStatus.textContent = error.message;
    }
  });
}

window.setInterval(refreshAirBoxes, 1000);
