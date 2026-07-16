# Air Traffic Scheduling System

A robust, dynamic simulation-based desktop application developed in C using the Raylib graphical user interface framework. This system mimics real-time runway allocation, flight scheduling, crew asset management, and air traffic control integration at a single airport.

---

## 📊 Project Architecture & Core Modules

The system breaks down into five integrated operational modules controlled by a central core processor:

1. **Flight Planning & Scheduling Module:** Handles incoming and outgoing flight registrations (supporting up to 50 concurrent flights) utilizing a strict priority hierarchy: `Emergency` > `International` > `Domestic`.
2. **Resource Allocation Module:** Dynamically maps aircraft to one of the 3 available airport runways while tracking crew assignment constraints.
3. **ATC & Real-Time Event Integration:** Monitors unexpected disruptions including weather delays, technical breakdowns, cancellations, and emergency landings.
4. **Optimization & Conflict Resolution Module:** Actively executes look-ahead checks to prevent runway slot overlaps and enforce buffer policies.
5. **GUI Display & Report Generation:** Renders live, graphical timetables using Raylib and outputs comprehensive slot allocations directly to `flight_and_slot_report.txt`.

---

## 🛠️ Operational Rules & Constraints

The program enforces the following simulated aviation compliance policies:
* **Runway Buffers:** A mandatory **15-minute window** must exist between consecutive takeoffs or landings on the same runway to prevent collisions.
* **Emergency Handling:** Immediate runway preemption. Emergency flights override all other domestic/international schedules.
* **Crew Compliance:** Flight crews have a maximum working ceiling of **8 hours of duty time per day** and require a mandatory **60-minute rest period** between flight pairings.
* **Weather Adjustments:** Weather delays trigger automatic calculation windows that shift subsequent non-emergency departures down the line.

---

## 🚀 Technical Implementation Highlights

* **Framework:** Raylib UI (Custom interactive graphical layouts and event handling logic)
* **Storage Engines:** C Structured Arrays / File Handling backend configuration (`.txt` slot persistence logging)
* **Algorithmic Strategies:** Priority Queue Simulation, Conflict Check Vectors, and Event-Driven State Rescheduling 

---


### Prerequisites
To build and run this simulator, verify your workstation has a standard C compiler and the Raylib library installed.
* **Windows:** GCC (via MinGW) or Clang configured with Raylib binaries
* **Mac/Linux:** `make`, `gcc`/`clang`, and `libraylib` development packages
