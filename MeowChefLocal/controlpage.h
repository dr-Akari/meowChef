#ifndef CONTROLPAGE_H
#define CONTROLPAGE_H

#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <Preferences.h>
#include <time.h>
#include <ArduinoJson.h>

// Define the web server port for the control page.
#define CONTROL_PORT 80

// Create a web server instance.
WebServer controlServer(CONTROL_PORT);

// Create a Servo object for the dispensing mechanism.
Servo mg995Servo;

// Define servo control pin and positions.
const int servoPin = 13;
const int neutralPosition = 90;  // Stop position.
const int dispensePosition = 0;  // Position for dispensing.
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;      // IST (UTC+5:30 = 5.5 * 3600)
const int   daylightOffset_sec = 0; 

// To ensure we trigger dispensing only once per scheduled minute
unsigned long lastScheduleCheck = 0;
unsigned long lastDispenseTrigger[10] = {0};  // support up to 10 entries

// Variables for servo timing control.
bool servoActive = false;
unsigned long servoStopTime = 0;

// Global variable to hold the current schedule (stored as JSON string).
String currentSchedule = "[]";

// Control Panel HTML updated with your provided CSS and layout.
const char* controlPageHTML = R"HTMLSTRING(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>MeowChef Control Panel</title>
    <style>
      /* Global body styling */
      body {
        font-family: Arial, sans-serif;
        background: #f7f7f7;
        margin: 0;
        padding: 20px;
      }
      /* Main container */
      .container {
        max-width: 800px;
        margin: 0 auto;
        background: #fff;
        border-radius: 8px;
        box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
        overflow: hidden;
      }
      /* Top section layout */
      .top-section {
        display: flex;
        flex-wrap: wrap;
        align-items: center;
      }
      .left-column,
      .right-column {
        flex: 1 1 50%;
        padding: 30px;
        box-sizing: border-box;
      }
      /* For smaller screens, stack the columns */
      @media (max-width: 600px) {
        .left-column,
        .right-column {
          flex: 1 1 100%;
          text-align: center;
        }
      }
      /* Limit dispense duration input width */
      #duration {
        max-width: 200px;
        margin: 0 auto;
        display: block;
      }
      /* Logo SVG scaling */
      .right-column svg.logo-svg {
        width: 100%;
        max-width: 200px;
        min-width: 120px;
        height: auto;
      }
      /* Header text styling */
      .header-text h1 {
        margin: 0;
        font-size: 2em;
        color: #333;
      }
      .header-text .subtitle {
        margin: 5px 0 0;
        color: #777;
        font-size: 1em;
      }
      /* Dividers */
      .inner-divider {
        border: none;
        border-top: 1px solid #ddd;
        margin: 20px 0;
      }
      .outer-divider {
        border: none;
        border-top: 1px solid #ddd;
        margin: 0;
      }
      /* Section styling for controls */
      .section {
        margin-bottom: 30px;
      }
      .section h2 {
        font-size: 1.5em;
        color: #333;
        margin: 0 0 10px;
      }
      label {
        display: block;
        margin: 10px 0 5px;
        color: #555;
      }
      input[type="number"],
      input[type="text"],
      input[type="time"] {
        padding: 10px;
        font-size: 1em;
        width: 100%;
        max-width: 200px;
        border: 1px solid #ccc;
        border-radius: 4px;
        box-sizing: border-box;
      }
      button {
        padding: 10px 20px;
        font-size: 1em;
        margin-top: 10px;
        cursor: pointer;
        border: none;
        border-radius: 4px;
        background-color: #fab616;
        color: #fff;
        transition: background 0.3s ease;
      }
      button:disabled {
        background-color: #6c757d;
        cursor: not-allowed;
      }
      button:hover:not(:disabled) {
        background-color: #d29d23;
      }
      /* Meal Schedule Section */
      .schedule-section {
        padding: 30px;
        box-sizing: border-box;
      }
      .schedule-header {
        display: flex;
        align-items: center;
        justify-content: space-between;
      }
      .schedule-header h2 {
        margin: 0;
        font-size: 1.5em;
        color: #333;
      }
      /* Edit Button Styles */
      .schedule-header button#edit-schedule-btn {
        background: none;
        border: none;
        cursor: pointer;
        padding: 5px;
        transition: background 0.3s ease;
      }
      .schedule-header button#edit-schedule-btn:hover {
        background-color: rgba(250, 182, 22, 0.2);
      }
      .schedule-header button#edit-schedule-btn svg.edit-icon {
        width: 24px;
        height: 24px;
        fill: #333;
        transition: fill 0.3s ease;
      }
      .schedule-header button#edit-schedule-btn:hover svg.edit-icon {
        fill: #d29d23;
      }
      .schedule-section p {
        margin: 10px 0 20px;
        color: #555;
      }
            /* System Clock Styling */
        #system-clock {
        text-align: center;
        font-size: 1.2em;
        color: #333;
        background: #f0f0f0;
        padding: 10px;
        border-radius: 4px;
        margin: 20px 30px;
      }
      /* Schedule row styling – keep inputs in a row */
      .schedule-row {
        display: flex;
        align-items: center;
        margin-bottom: 10px;
        flex-wrap: nowrap;
      }
      .schedule-row input.schedule-time,
      .dose-input-wrapper input.schedule-dose {
        height: 40px;
      }
      .schedule-row input.schedule-time {
        width: 120px;
        margin-right: 10px;
      }
      .dose-input-wrapper {
        position: relative;
        width: 80px;
        margin-right: 10px;
      }
      .dose-input-wrapper input.schedule-dose {
        width: 100%;
        padding-right: 25px;
      }
      .dose-input-wrapper .input-suffix {
        position: absolute;
        right: 8px;
        top: 50%;
        transform: translateY(-50%);
        pointer-events: none;
        color: #aaa;
        font-size: 0.9em;
      }
      /* Remove entry button styling */
      .schedule-row button.remove-entry-btn {
        background-color: #dc3545;
        padding: 0 15px;
        height: 40px;
        display: flex;
        align-items: center;
        justify-content: center;
        border-radius: 4px;
        border: none;
        cursor: pointer;
        margin-top: 0;
      }
      .schedule-row button.remove-entry-btn:hover {
        background-color: #c82333;
      }
      /* Schedule controls (Add and Save buttons) */
      .schedule-controls {
        margin-top: 10px;
      }
      /* In view mode, hide edit controls */
      .schedule-section:not(.editing) .schedule-controls,
      .schedule-section:not(.editing) .schedule-row button.remove-entry-btn {
        display: none;
      }
      /* Notification styling */
      #notification {
        margin: 20px 30px 30px;
        padding: 10px 20px;
        border-radius: 4px;
        font-size: 1em;
        opacity: 0;
        transition: opacity 0.5s ease;
      }
      #notification.show {
        opacity: 1;
      }
      #notification.info {
        background-color: #d9edf7;
        color: #31708f;
      }
      #notification.success {
        background-color: #dff0d8;
        color: #3c763d;
      }
      #notification.error {
        background-color: #f2dede;
        color: #a94442;
      }
      /* Error log container styling */
      #error-log {
        margin: 20px 30px;
        padding: 10px 20px;
        border-radius: 4px;
        background-color: #f8d7da;
        color: #721c24;
        display: none;
      }
      #error-log h3 {
        margin-top: 0;
      }
    </style>
    <script>
        // Define a helper function to add a schedule row.
        function addScheduleRow(time = "", dose = 3) {
          const scheduleEntries = document.getElementById("schedule-entries");
          const row = document.createElement("div");
          row.classList.add("schedule-row");
  
          const timeInput = document.createElement("input");
          timeInput.type = "time";
          timeInput.classList.add("schedule-time");
          timeInput.value = time; // prepopulate if provided
  
          const doseWrapper = document.createElement("div");
          doseWrapper.classList.add("dose-input-wrapper");
  
          const doseInput = document.createElement("input");
          doseInput.type = "number";
          doseInput.classList.add("schedule-dose");
          doseInput.placeholder = "Dose";
          doseInput.value = dose;
          doseInput.min = 1;
          doseInput.step = 1;
  
          const suffixSpan = document.createElement("span");
          suffixSpan.classList.add("input-suffix");
          suffixSpan.textContent = "s";
  
          doseWrapper.appendChild(doseInput);
          doseWrapper.appendChild(suffixSpan);
  
          const removeBtn = document.createElement("button");
          removeBtn.textContent = "–";
          removeBtn.type = "button";
          removeBtn.classList.add("remove-entry-btn");
          removeBtn.addEventListener("click", () => {
            row.remove();
          });
  
          row.appendChild(timeInput);
          row.appendChild(doseWrapper);
          row.appendChild(removeBtn);
          scheduleEntries.appendChild(row);
        }
  
        // Fetch schedule from the server before rendering schedule rows.
        document.addEventListener("DOMContentLoaded", function () {
          // Fetch stored schedule from /getSchedule endpoint.
          fetch("/getSchedule")
            .then(response => response.json())
            .then(data => {
              console.log("GET /getSchedule returned:", data);
              const scheduleEntries = document.getElementById("schedule-entries");
              // Clear any existing rows.
              scheduleEntries.innerHTML = "";
              // Check if data is an array with entries.
              if (data && Array.isArray(data) && data.length > 0) {
                data.forEach(entry => {
                  addScheduleRow(entry.time, entry.dose);
                });
              } else {
                // If no valid schedule is stored, add four default entries.
                addScheduleRow("11:00", 3);
                addScheduleRow("14:00", 3);
                addScheduleRow("18:00", 3);
                addScheduleRow("04:00", 3);
              }
            })
            .catch(error => {
              console.error("Error fetching schedule:", error);
              // On error, fallback to default entries.
              const scheduleEntries = document.getElementById("schedule-entries");
              scheduleEntries.innerHTML = "";
              addScheduleRow("11:00", 3);
              addScheduleRow("14:00", 3);
              addScheduleRow("18:00", 3);
              addScheduleRow("04:00", 3);
            });
        });
      </script>
  </head>
  <body>
    <div class="container">
      <!-- Top Section -->
      <div class="top-section">
        <!-- Left Column: Header & Dispense Control -->
        <div class="left-column">
          <div class="header-text">
            <h1>MeowChef</h1>
            <p class="subtitle">Control Panel</p>
          </div>
          <hr class="inner-divider" />
          <div id="dispense-section" class="section">
            <h2>Dispense Control</h2>
            <label for="duration">Dispense Duration (ms):</label>
            <input type="number" id="duration" value="3000" min="100" step="100" />
            <button id="dispense-btn">Dispense Now</button>
          </div>
        </div>
        <!-- Right Column: Logo -->
        <div class="right-column">
            <svg class="logo-svg" viewBox="0 0 84 85" fill="none" xmlns="http://www.w3.org/2000/svg">
                <path d="M46.75 59.5H75.0833L67.0919 68H54.015L46.75 59.5Z" fill="#FBF0D0"/>
                <circle cx="60.9167" cy="75.0833" r="9.91667" fill="#FFC547"/>
                <path d="M57.0667 82.075L56.741 81.6407L56.2369 81.8423C56.0253 81.927 55.8989 81.9226 55.8288
                 81.907C55.7604 81.8918 55.6787 81.8512 55.5808 81.7426C55.3594 81.4971 55.1558 81.0155 55.0054
                  80.3065C54.7112 78.9187 54.7083 77.1213 54.7083 76C54.7083 74.9677 54.9476 74.4618 55.1727
                   74.2501C55.353 74.0806 55.6708 73.9702 56.276 74.172C56.9753 74.4051 57.4506 74.6731 57.7636
                    75.0568C58.0674 75.4293 58.2917 76.0026 58.2917 77C58.2917 77.2661 58.2979 77.5876 58.3048
                     77.9435C58.3284 79.1633 58.3599 80.7874 58.1752 81.969C58.1168 82.3424 58.0424 82.6268 57.9593
                      82.8187C57.9367 82.8711 57.9165 82.9094 57.9001 82.9369C57.8875 82.9296 57.8731 82.9208 57.8569
                       82.9102C57.6843 82.7971 57.4223 82.5491 57.0667 82.075ZM57.9748 82.9708C57.9749 82.971 57.9726
                        82.9707 57.9679 82.9693C57.9723 82.9698 57.9747 82.9706 57.9748 82.9708Z" fill="white" stroke="white" stroke-width="1.41667"/>
                <circle cx="60.9167" cy="75.0833" r="9.20833" stroke="white" stroke-width="1.41667"/>
                <ellipse cx="61.125" cy="54.7565" rx="22.875" ry="7.86328" fill="#FFC547"/>
                <rect x="38.25" y="31.1667" width="45.75" height="24.3047" fill="#FFC547"/>
                <ellipse cx="60.9167" cy="31.875" rx="22.6667" ry="7.79167" fill="#FBF0D0"/>
                <path d="M44 49C44 53.4183 42.7614 57 40 57C37.2386 57 35 53.4183 35 49C35 44.5817 37.2386 41 40 41C42.7614 41 44 44.5817 44 49Z" fill="#FFC547"/>
                <ellipse cx="30.5068" cy="56.1162" rx="1.09325" ry="5.01359" transform="rotate(-49.1332 30.5068 56.1162)" fill="#FFC547"/>
                <rect x="34" y="41" width="6" height="15" fill="#FFC547"/>
                <path d="M32.5833 80.75C32.5833 82.3148 26.5578 83.5833 19.125 83.5833C11.6922 83.5833 5.66666 82.3148 5.66666 80.75C5.66666 79.1852 11.6922 77.9167 19.125 77.9167C26.5578 77.9167 32.5833 79.1852 32.5833 80.75Z" fill="#FFC547"/>
                <path d="M0 70.8333H38.25L32.5833 80.75H5.66667L0 70.8333Z" fill="#FFC547"/>
                <ellipse cx="19.125" cy="70.8333" rx="19.125" ry="4.25" fill="#FBF0D0"/>
                <ellipse cx="34.5" cy="48.5" rx="3.5" ry="7.5" fill="#FFC547"/>
                <path d="M26.8121 52.8901L32 43.5L38.5 56.5L34.4053 59.3649L26.8121 52.8901Z" fill="#FFC547"/>
                <path d="M83 34.5833V7.5" stroke="#FFC547" stroke-width="1.41667"/>
                <path d="M39 35.3333L39 7" stroke="#FFC547" stroke-width="1.41667"/>
                <ellipse cx="60.9167" cy="7.79167" rx="22.6667" ry="7.79167" fill="#FBF0D0"/>
                <path d="M41 42C41 42 44 43.5517 44 49.7586C44 55.9655 41 57 41 57" stroke="white" stroke-width="0.8" stroke-linecap="round"/>
                <ellipse cx="56" cy="76.5" rx="1" ry="1.5" fill="#FFC547"/>
                <path d="M63.4989 48.8864C64.2334 48.788 64.7623 47.9618 64.6806 47.0408C64.5987 46.12 63.9371 45.4534 63.2028 45.5518C62.4684 45.6505 61.9394 46.4766 62.0212 47.3975C62.103 48.3184 62.7646 48.985 63.4989 48.8864Z" fill="white"/>
                <path d="M61.7265 51.3034C62.3565 50.9231 62.4811 49.992 62.0049 49.2232C61.5285 48.4547 60.6316 48.1399 60.0014 48.5198C59.3715 48.9003 59.2469 49.8314 59.7232 50.6001C60.1996 51.3686 61.0964 51.6835 61.7265 51.3034Z" fill="white"/>
                <path d="M66.8258 48.7281C67.5595 48.757 68.2293 48.027 68.3222 47.0979C68.4148 46.1684 67.8955 45.3921 67.1617 45.3633C66.4282 45.3347 65.7583 46.0649 65.6655 46.9939C65.5728 47.9233 66.0923 48.6995 66.8258 48.7281Z" fill="white"/>
                <path d="M70.3297 48.0285C69.7038 47.7083 68.8026 48.1085 68.3168 48.9228C67.8311 49.7374 67.9448 50.6572 68.5708 50.9777C69.1966 51.2979 70.0979 50.8977 70.5836 50.0832C71.0691 49.2688 70.9556 48.349 70.3297 48.0285Z" fill="white"/>
                <path d="M66.5412 50.0358C66.2407 49.6112 65.7293 49.3423 65.1402 49.3703C64.5513
                 49.3983 64.0364 49.7162 63.7307 50.1693C63.3929 50.6703 63.2654 51.1685 62.9358 51.5226C62.6064
                  51.8767 61.2454 51.9893 61.1354 53.539C61.0577 54.6336 62.0488 55.4745 63.1899 55.4202C64.0573
                   55.3789 64.6028 55.0634 65.1067 55.0393C65.6105 55.0152 66.1526 55.2791 67.0199 55.2378C68.161
                    55.1835 69.1627 54.2479 69.0979 53.1601C69.0064 51.6199 67.6458 51.6369 67.3203 51.314C66.9949
                     50.991 66.8732 50.5047 66.5412 50.0358Z" fill="white"/>
                </svg>
                
        </div>
      </div>

      <hr class="outer-divider" />

      <!-- Meal Schedule Section -->
      <div class="schedule-section" id="schedule-section">
        <div class="schedule-header">
          <h2>Meal Schedule</h2>
          <button id="edit-schedule-btn" title="Edit Schedule">
            <svg class="edit-icon" viewBox="0 0 24 24">
              <path d="M3 17.25V21h3.75L17.81 9.94l-3.75-3.75L3 17.25zM20.71 7.04a1.003 1.003 0 000-1.42l-2.34-2.34a1.003 1.003 0 00-1.42 0l-1.83 1.83 3.75 3.75 1.84-1.82z"/>
            </svg>
          </button>
        </div>
        <p>Note: A 3‑second dose dispenses approximately 20–25g.</p>

        <div id="schedule-entries">
          <!-- Schedule rows will be rendered here -->
        </div>
        <div class="schedule-controls">
          <button id="add-entry-btn">+ Add Entry</button>
          <button id="schedule-btn">Save Schedule</button>
        </div>
      </div>
      <div id="system-clock" style="text-align:center; font-size:1.2em; margin-top:20px;"></div>
      <!-- Notification Area -->
      <div id="notification"></div>
      <!-- Error Log Container -->
      <div id="error-log">
        <h3>Error Logs:</h3>
        <div id="error-log-content"></div>
      </div>    
    </div>


    <script>
    // Function to update the system clock display
    function updateDeviceClock() {
    fetch("/getTime")
      .then(response => response.text())
      .then(timeText => {
        document.getElementById("system-clock").textContent = "System Clock: " + timeText;
      })
      .catch(error => {
        console.error("Error fetching device time:", error);
      });
  }
      // Update the device clock every second.
      setInterval(updateDeviceClock, 1000);
      updateDeviceClock();

      // Default stored schedule (as provided)
      const storedSchedule = "[{\"time\":\"11:00\",\"dose\":3},{\"time\":\"14:00\",\"dose\":3},{\"time\":\"18:00\",\"dose\":3},{\"time\":\"04:00\",\"dose\":3}]";
      let scheduleData = JSON.parse(storedSchedule);
      let editMode = false; // Start in view mode

      const dispenseBtn = document.getElementById("dispense-btn");
      const scheduleBtn = document.getElementById("schedule-btn");
      const addEntryBtn = document.getElementById("add-entry-btn");
      const editScheduleBtn = document.getElementById("edit-schedule-btn");
      const scheduleEntries = document.getElementById("schedule-entries");
      const scheduleSection = document.getElementById("schedule-section");
      const notification = document.getElementById("notification");

      // Utility to show notifications
      function showNotification(message, type) {
        notification.textContent = message;
        notification.className = type + " show";
        setTimeout(() => {
          notification.textContent = "";
          notification.className = "";
        }, 5000);
      }

      // Utility to log errors
      function logError(message) {
        const errorLogContainer = document.getElementById("error-log");
        const errorLogContent = document.getElementById("error-log-content");
        errorLogContainer.style.display = "block";
        const logItem = document.createElement("div");
        logItem.textContent = message;
        errorLogContent.appendChild(logItem);
      }

      // Render schedule rows based on scheduleData and editMode
      function renderScheduleRows() {
        scheduleEntries.innerHTML = "";
        scheduleData.forEach((entry, index) => {
          const row = document.createElement("div");
          row.classList.add("schedule-row");

          const timeInput = document.createElement("input");
          timeInput.type = "time";
          timeInput.classList.add("schedule-time");
          timeInput.disabled = !editMode;
          if (editMode) {
            timeInput.addEventListener("change", () => {
                scheduleData[index].time = timeInput.value;
            });
            }

          const doseWrapper = document.createElement("div");
          doseWrapper.classList.add("dose-input-wrapper");

          const doseInput = document.createElement("input");
          doseInput.type = "number";
          doseInput.classList.add("schedule-dose");
          doseInput.value = entry.dose;
          doseInput.min = 1;
          doseInput.step = 1;
          doseInput.disabled = !editMode;
          if (editMode) {
            doseInput.addEventListener("change", () => {
                scheduleData[index].dose = doseInput.value;
            });
            }


          const suffixSpan = document.createElement("span");
          suffixSpan.classList.add("input-suffix");
          suffixSpan.textContent = "s";

          doseWrapper.appendChild(doseInput);
          doseWrapper.appendChild(suffixSpan);

          row.appendChild(timeInput);
          row.appendChild(doseWrapper);

          if (editMode) {
            const removeBtn = document.createElement("button");
            removeBtn.textContent = "–";
            removeBtn.type = "button";
            removeBtn.classList.add("remove-entry-btn");
            removeBtn.addEventListener("click", () => {
              scheduleData.splice(index, 1);
              renderScheduleRows();
            });
            row.appendChild(removeBtn);
          }

          scheduleEntries.appendChild(row);
        });
      }

      // Toggle edit mode for schedule section
      editScheduleBtn.addEventListener("click", () => {
        editMode = !editMode;
        if (editMode) {
          scheduleSection.classList.add("editing");
          editScheduleBtn.title = "Done Editing";
        } else {
          scheduleSection.classList.remove("editing");
          editScheduleBtn.title = "Edit Schedule";
        }
        renderScheduleRows();
      });

      // Add new schedule row (only allowed in edit mode)
      addEntryBtn.addEventListener("click", () => {
        if (!editMode) return;
        scheduleData.push({ time: "", dose: 3 });
        renderScheduleRows();
      });

      // Save schedule button handler (only active in edit mode)
scheduleBtn.addEventListener("click", () => {
  if (!editMode) return;
  fetch("/saveSchedule?schedule=" + encodeURIComponent(JSON.stringify(scheduleData)))
    .then((response) => {
      if (!response.ok) {
        return response.text().then((text) => {
          const errorMessage = text.includes("<html")
            ? "Couldn't save schedule. Please try again."
            : text;
          logError("Schedule Error: " + errorMessage);
          throw new Error(errorMessage);
        });
      }
      return response.text();
    })
    .then((data) => {
      showNotification(data, "success");
      scheduleBtn.disabled = false;
      // Exit edit mode after saving
      editMode = false;
      scheduleSection.classList.remove("editing");
      editScheduleBtn.title = "Edit Schedule";
      renderScheduleRows();
    })
    .catch((error) => {
      console.error("Error:", error);
      showNotification(error.message, "error");
      scheduleBtn.disabled = false;
    });
});


      // Dispense Now button event listener
      dispenseBtn.addEventListener("click", () => {
        const duration = document.getElementById("duration").value;
        if (duration < 100) {
          showNotification("Duration must be at least 100 ms.", "error");
          logError("Dispense Error: Duration must be at least 100 ms.");
          return;
        }
        dispenseBtn.disabled = true;
        showNotification("Dispensing... please wait.", "info");

        fetch("/dispense?duration=" + duration)
          .then((response) => {
            if (!response.ok) {
              return response.text().then((text) => {
                const errorMessage = text.includes("<html")
                  ? "Couldn't dispense. Please try again."
                  : text;
                logError("Dispense Error: " + errorMessage);
                throw new Error(errorMessage);
              });
            }
            return response.text();
          })
          .then((data) => {
            showNotification(data, "success");
            dispenseBtn.disabled = false;
          })
          .catch((error) => {
            console.error("Error:", error);
            showNotification(error.message, "error");
            dispenseBtn.disabled = false;
          });
      });

      // Initial render of schedule rows (view mode, showing defaults)
      renderScheduleRows();
    </script>
  </body>
</html>
)HTMLSTRING";

// Handler for control page root.
void handleControlRoot() {
  controlServer.send(200, "text/html", controlPageHTML);
}

// Handler for dispensing command.
void handleDispense() {
  String durStr = controlServer.arg("duration");
  unsigned long duration = 3000; // default duration.
  if (durStr.length() > 0) {
    duration = durStr.toInt();
    if (duration == 0) duration = 3000;
  }
  mg995Servo.write(dispensePosition);
  servoActive = true;
  servoStopTime = millis() + duration;
  controlServer.send(200, "text/plain", "Dispensing for " + String(duration) + " ms");
}

// Function to check the schedule and trigger dispensing
void checkSchedule() {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);

  // Format current time as "HH:MM"
  char currentTime[6];
  strftime(currentTime, sizeof(currentTime), "%H:%M", &timeinfo);
  Serial.print("Checking schedule at: ");
  Serial.println(currentTime);

  // Parse the current schedule JSON
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, currentSchedule);
  if (error) {
    Serial.print("Failed to parse schedule: ");
    Serial.println(error.f_str());
    return;
  }
  JsonArray scheduleArray = doc.as<JsonArray>();

  // Static array to track last triggered minute for each schedule entry
  static unsigned long lastTriggered[10] = {0};
  int index = 0;
  for (JsonObject entry : scheduleArray) {
    const char* schedTime = entry["time"]; // Should be in "HH:MM" format
    int dose = entry["dose"];              // Dose in seconds

    if (strcmp(schedTime, currentTime) == 0) {
      // Calculate the current minute (since epoch)
      unsigned long currentMinute = now / 60;
      // If not already triggered this minute, trigger dispensing
      if (lastTriggered[index] != currentMinute) {
        Serial.print("Triggering dispense for schedule at: ");
        Serial.println(schedTime);
        lastTriggered[index] = currentMinute;
        int durationMs = dose * 1000;  // Convert seconds to milliseconds
        
        // Activate the servo for dispensing
        mg995Servo.write(dispensePosition);
        servoActive = true;
        servoStopTime = millis() + durationMs;
      }
    }
    index++;
  }
}

void handleGetTime() {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  char timeString[9]; // Format: HH:MM:SS
  strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeinfo);
  controlServer.send(200, "text/plain", timeString);
}

// Load schedule from NVS on startup.
void loadSchedule() {
  Preferences preferences;
  preferences.begin("dispense", true);
  String storedSchedule = preferences.getString("schedule", "");
  // If no schedule is stored or it's empty, set default values.
  if (storedSchedule.length() == 0 || storedSchedule == "[]") {
      storedSchedule = "[{\"time\":\"11:00\",\"dose\":3},{\"time\":\"14:00\",\"dose\":3},{\"time\":\"18:00\",\"dose\":3},{\"time\":\"04:00\",\"dose\":3}]";
      // Save the default schedule back to NVS.
      preferences.end();
      preferences.begin("dispense", false);
      preferences.putString("schedule", storedSchedule);
  }
  currentSchedule = storedSchedule;
  Serial.println("Loaded schedule: " + currentSchedule);
  preferences.end();
}


// Handler for saving the schedule.
void handleSaveSchedule() {
  String dispenseSchedule = controlServer.arg("schedule");
  Preferences preferences;
  preferences.begin("dispense", false);
  preferences.putString("schedule", dispenseSchedule);
  preferences.end();
  currentSchedule = dispenseSchedule;  // update global variable.
  controlServer.send(200, "text/plain", "Schedule saved: " + dispenseSchedule);
}


// Handler to get the current schedule.
void handleGetSchedule() {
  Serial.println("GET /getSchedule called, returning schedule: " + currentSchedule);
  controlServer.send(200, "application/json", currentSchedule);
}


// Setup function for the control page.
void setupControlPage() {
  mg995Servo.attach(servoPin);
  mg995Servo.write(neutralPosition);
  
  controlServer.on("/", HTTP_GET, handleControlRoot);
  controlServer.on("/dispense", HTTP_GET, handleDispense);
  controlServer.on("/saveSchedule", HTTP_GET, handleSaveSchedule);
  controlServer.on("/getSchedule", HTTP_GET, handleGetSchedule);
  controlServer.on("/getTime", HTTP_GET, handleGetTime);

  controlServer.begin();
  Serial.println("Control page server started on port " + String(CONTROL_PORT));
}

#endif // CONTROLPAGE_H
