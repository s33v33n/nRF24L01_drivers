# Secure nRF24L01+ Framework

Secure RF communication framework for **nRF24L01+** transceivers on Linux platform (exam. Raspberry Pi). The project integrates a custom Linux SPI kernel driver with authenticated encryption (**ChaCha20-Poly1305**), providing a secure star-topology wireless network up to 256 devices.

---

* **Star Network Topology:**
  * Single **Master** (`NODE_ID = 0`).
  * Multiple **Slaves** (`NODE_ID = 1..N`).
  * Communication is strictly point-to-point between Master and Slaves (Slaves do not communicate directly with each other).
* **Maximum User Payload:** **11 bytes** per 32-byte radio packet.
* **Authenticated Encryption (AEAD):** Based on **ChaCha20-Poly1305**.
* **Dedicated Linux Kernel Driver:** Custom SPI driver with Device Tree Overlay support, non-blocking `poll()` mechanism, and full `ioctl` configuration interface.

---

## Radio Packet Structure (32 bytes)

The fixed 32-byte hardware payload of the nRF24L01+ is structured as follows:

```text
|  Sender ID (1 B)  | Replay Counter (4 B) |  Encrypted Payload (11 B)   |  Poly1305 Tag (16 B)  |
```

---

## Repository Structure

```text
├── driver/           # Linux kernel driver (nrf24l01.ko) and Device Tree source (.dts/.dtbo)
├── encryption/       # ChaCha20-Poly1305 cryptographic implementation
├── framework/        # High-level library API (nrf_framework.c, nrf_packet.c)
├── include/          # Internal header files (IOCTL definitions)
├── user_config/      # User configurations (hardware_config.h, user_config.h)
├── examples/         # Application examples (master.c, slave.c)
├── tools/            # Utilities (auto_config.sh generator, data_validator.c)
├── Makefile          # Main build, permanent install, and packaging rules
└── build.sh          # Interactive build script (validation, compilation, loading)
```

---

## Building and Installation

The script guides you through the process:
1. **Optional default Configuration Generation :** Generates a random 256-bit network key and nodes addresses. (The user must fill in `user_config.h`, or if preferred, the script will automatically generate everything for them).
2. **Automated Validation:** Validates network settings.
3. **Device Tree Compilation:** Generates and compiles the overlay based on **your** hardware SPI pin definitions. Hardware connections depend on your device, and `driver/nrf24l01.dts.template` must be created/adjusted according to wiring in `hardware_config.h`.
4. **Target Node Selection:** Prompts for `NODE_ID`:
   * `0` – compiles the **master** binary.
   * `1..N` – compiles the **slave** binary for the specified node ID.
5. **Installation and Immediate Activation:**
   * Permanent setup across reboots.
   * Dynamic loading of the Device Tree overlay and kernel module in the running system (no reboot needed).

You can pass the target `NODE_ID` to build binaries, default is 0:
```bash
./build.sh 0   # Build and configure as Master
./build.sh 1   # Build and configure as Slave (Node ID 1)
```
...and run your program
```bash
./master  # Run Master (Node ID 0)
./slave   # Run Slave (Node ID 1)
```

## Framework API (`nrf_framework.h`)

User-space applications interact with the radio network via high-level API:

```c
#include "framework/nrf_framework.h"

// 1. Initialization and Cleanup
int nrf_init(void);    // Opens /dev/nrf24l01_* and configures radio referring to node ID config 
int nrf_close(void);   // Closes device descriptor cleanly

// 2. Transmission (handles AEAD encryption and tagging)
int nrf_master_send(uint8_t target_node_id, char *message); // Master sends to a specific Slave
int nrf_node_send(char *message);                           // Slave sends back to Master

// 3. Reception (handles AEAD decryption and authentication tag check)
int nrf_receive(uint8_t *sender_id, char *out_message, size_t max_len);

// 4. Runtime Radio Configuration (optional)
int nrf_set_channel(uint8_t channel);
int nrf_set_power(uint8_t power);
int nrf_set_speed(uint8_t speed);
```

During software development, the user also has access to inspect the node list (`nrf_nodes` in `user_config/user_config.h`) to view configured node IDs and settings.