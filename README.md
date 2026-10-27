# 🏎️ GrandPrix — Autonomous Racing Pilot

An **autonomous racing agent written in C**, integrated with the **GrandPrix Qt/C++ race simulator**.

The goal is simple: **reach the finish line as efficiently as possible.**

The challenge is not.

At every turn, the pilot must choose an acceleration while accounting for **inertia, speed limits, terrain, fuel consumption, limited boosts, track boundaries, and other cars**.

The `omama` pilot solves this using a combination of **A\* path planning, physics-aware move validation, fuel optimization, and a safety fallback strategy**.

---

## ✨ Features

- **A\* path planning** that searches for a route to the finish while considering the car's position and velocity
- **Physics-aware navigation** with acceleration, inertia, velocity, and maximum-speed constraints
- **Fuel-aware planning** that penalizes expensive accelerations and high-speed movement
- **Terrain handling**, including reduced speed and additional cost when driving through sand
- **Boost management** with a limited number of high-acceleration moves
- **Full trajectory validation** rather than checking only the destination cell
- **Collision avoidance** against the other racers' reported positions
- **Adaptive fallback strategy** when the A\* move cannot safely be executed
- **Real-time replanning** at every race turn
- **Communication with the simulator** through a lightweight `stdin`/`stdout` protocol

---

## 🧠 How It Works

For every turn, the race manager sends the current positions of the three cars to the pilot.

The pilot then:

1. Reconstructs its current velocity from its previous and current positions.
2. Runs an **A\* search** toward the finish line.
3. Evaluates candidate accelerations while respecting:
   - Track boundaries
   - Obstacles
   - Maximum velocity
   - Sand speed restrictions
   - Remaining fuel
   - Available boosts
4. Reconstructs the first acceleration of the best path.
5. Validates that movement against the current positions of the other cars.
6. If the A\* solution is unavailable or unsafe, switches to a **safe acceleration strategy**.
7. Sends the selected acceleration back to the GrandPrix race manager.

This process repeats every turn, allowing the pilot to continuously adapt to the race.

---

## 🔎 A\* Search

The planner does not treat the car as a point that can instantly move between adjacent cells.

Instead, the search incorporates the vehicle's **inertia**:

```text
new_velocity = current_velocity + acceleration
new_position = current_position + new_velocity
```

The A\* evaluation considers several objectives:

```text
cost = turn cost
     + fuel consumption
     + sand penalty
     + heuristic toward the finish
```

The heuristic combines the distance to the finish with the projection of the current velocity toward it.

As a result, the pilot favors states that are not only close to the goal, but are also already moving in a useful direction.

A priority queue implemented as a **binary min-heap** is used to efficiently retrieve the next best A\* state.

---

## 🛡️ Safety Fallback

A valid A\* path does not necessarily guarantee that the first move is still safe once the other racers are considered.

After planning, the selected acceleration is validated against the live race state.

If it is rejected—or if A\* cannot find a valid path—the pilot evaluates the currently legal accelerations using a separate scoring strategy.

The fallback considers factors such as:

- Progress toward the finish
- Current and target speed
- Acceleration cost
- Remaining fuel
- Sand
- Braking
- Coasting
- Stopping risk

This provides a second layer of decision-making instead of simply stopping when the main planner fails.

---

## ⛽ Physics & Fuel

The pilot maintains its own representation of the car:

```c
typedef struct {
    int x;
    int y;
    int vx;
    int vy;
    int gas;
    int boosts;
} Car;
```

Fuel consumption depends on both the requested acceleration and the car's current speed.

Driving through sand also increases the cost and imposes a much lower speed limit.

The implementation currently uses:

| Parameter | Value |
| --- | ---: |
| Maximum normal speed | `5` |
| Maximum sand speed | `1` |
| Initial boosts | `5` |

**Boost moves** allow larger acceleration values than normal driving.

---

## 🛣️ Trajectory Validation

A move is not considered safe just because its final coordinate lies on the track.

The `follow_line` module traverses the cells crossed between the current position and the next position.

This allows the pilot to reject trajectories that:

- Cross non-drivable cells
- Leave the map
- Pass through sand at excessive speed
- Intersect another car's reported position

This is particularly important at high speed, where a car can travel across multiple map cells during a single turn.

---

## 🏗️ Project Structure

```text
GrandPrix/
├── src/                 # Qt/C++ race manager implementation
├── include/             # Race manager headers
├── ui/                  # Qt Designer interfaces
├── res/                 # GUI resources
├── tracks/              # Available race tracks
├── drivers/             # Executable pilots loaded by GrandPrix
├── omama/               # Autonomous racing pilot
│   ├── main.c           # Main race loop
│   ├── communication.c  # Simulator I/O
│   ├── map_utils.c      # Track utilities
│   ├── physics.c        # Movement, fuel & collision validation
│   ├── strategy.c       # A* planner & fallback strategy
│   └── Makefile
├── follow_line/         # Trajectory traversal utilities
├── test_pilot/          # Basic reference/test pilot
├── GrandPrix.pro        # Qt project configuration
└── Makefile
```

---

## 🧰 Tech Stack

### Autonomous Pilot

- C
- A\* search
- Binary heap / priority queue
- Graph search
- Heuristic optimization
- Physics simulation
- Collision detection

### Race Simulator

- C++11
- Qt
- Qt Widgets
- qmake

### Build Tools

- GCC / G++
- Make
- qmake

---

## 🚀 Build & Run

### Requirements

You will need:

- GCC / G++
- Make
- Qt
- qmake
- Qt Widgets

### 1. Build GrandPrix

From the repository root:

```bash
qmake
make
```

### 2. Build the Autonomous Pilot

```bash
make -C ./omama install
```

This builds the pilot and copies the executable into:

```text
./drivers/omama
```

GrandPrix automatically discovers executable pilots placed in the `drivers/` directory.

### 3. Start the Simulator

```bash
./GrandPrix
```

Select a track, choose `omama` as one of the drivers, and start the race.

Alternatively, once the GrandPrix application has already been compiled:

```bash
make -C ./omama try
```

This builds the pilot, installs it, and launches the simulator.

---

## 🗺️ Track Representation

The pilot parses the track directly from the map sent by GrandPrix.

Some important map cells are:

| Symbol | Meaning |
| :---: | --- |
| `#` | Drivable track |
| `~` | Sand |
| `=` | Finish line |
| `1` | Car/start position 1 |
| `2` | Car/start position 2 |
| `3` | Car/start position 3 |

The map can contain different terrain types that directly affect the pilot's planning strategy.

---

## 📡 Driver Communication

The autonomous driver runs as a separate process from the GrandPrix simulator.

At startup, it receives information about the track, including its dimensions, initial fuel, and map layout.

During the race, GrandPrix sends the positions of the cars.

The pilot responds with its selected acceleration:

```text
dvx dvy
```

This architecture makes it possible to implement completely different racing strategies without modifying the simulator itself.

---

## 🎯 What This Project Explores

GrandPrix is primarily an exercise in applying algorithms to a **dynamic environment** rather than simply finding the shortest geometric path.

The autonomous pilot combines concepts from:

**Graph Search · Heuristics · Data Structures · Optimization · Simulation · Collision Detection · Resource Management · Real-Time Decision Making**

A good route must not only reach the finish—it must be **physically executable, fuel-efficient, terrain-aware, and safe at the current velocity**.

---

## 🙏 Acknowledgements

The GrandPrix race-management framework is part of an **ENSICAEN engineering project**.

**Original race manager:**

- **Julien Gosme** — original implementation
- **Gilles Lebrun** — modifications
- **Sébastien Fourey** — modifications and maintenance

The autonomous driver contained in `omama/` extends the simulator with the path-planning and racing strategy described above.
