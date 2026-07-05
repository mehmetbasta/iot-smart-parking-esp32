\## Features



\- \*\*Dual barrier control\*\* — separate entry and exit gates, each with an SG90 servo motor

\- \*\*Real-time occupancy detection\*\* — IR sensors at every parking spot

\- \*\*Entrance LCD display\*\* — shows the number of available/occupied spots at a glance

\- \*\*Per-spot LED status indicators\*\* (WS2812B addressable LED strips):

&#x20; - 🔴 Red — occupied

&#x20; - 🟢 Green — available

&#x20; - 🔵 Blue — reserved

\- \*\*Web dashboard (Firebase)\*\* — view live spot availability from anywhere

\- \*\*Remote reservation system\*\* — users reserve an open spot via the website and 

&#x20; receive a unique 4-digit PIN

\- \*\*Keypad-based entry\*\* — PIN required only when no unreserved spot remains

\- \*\*Automatic misparking correction\*\* — if a driver parks in the wrong reserved 

&#x20; spot, the system detects it, flags it visually, and automatically reassigns 

&#x20; the affected reservation to a still-open spot so the other driver isn't 

&#x20; penalized for someone else's mistake



\## How It Works



1\. IR sensors continuously monitor occupancy at every spot, synced to Firebase 

&#x20;  in real time and reflected on the web dashboard and the WS2812B LEDs

2\. Users reserve an open spot through the website and receive a 4-digit PIN 

&#x20;  tied to that specific spot

3\. If at least one truly unreserved spot remains, the barrier opens without a 

&#x20;  PIN; once all remaining spots are occupied or reserved, the LCD requires a 

&#x20;  PIN before the barrier will open

4\. At the barrier, the driver enters their PIN; if it matches an active 

&#x20;  reservation, the barrier opens

5\. The system tracks which spot each entering PIN was assigned to. If the 

&#x20;  driver then parks in a different reserved spot than their own:

&#x20;  - The LCD displays a misparking warning

&#x20;  - The LED above the spot they mistakenly occupied flashes as an alert

&#x20;  - That spot's status updates to occupied

&#x20;  - The reservation that was originally tied to the now-occupied spot is 

&#x20;    automatically transferred to the spot the misparked driver was 

&#x20;    originally assigned to (which is still open) — so the second driver 

&#x20;    isn't penalized for someone else's mistake, and simply sees their new 

&#x20;    spot number on the LCD when they arrive and enter their own PIN



\## Tech Stack



\- ESP32 (Arduino framework, C/C++)

\- Google Firebase Realtime Database

\- IR sensors (occupancy detection)

\- WS2812B addressable LED strips (per-spot status)

\- 16x2 LCD display (entrance status)

\- 4x3 Keypad (PIN entry)

\- 2x SG90 servo motors (entry \& exit barriers)



\## Project Status



Completed as a graduation project.

