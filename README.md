# OperatingSystemSimulator

## Team Members
- Cash Bailey
- Alejandro Castineyra
- Sylvia Ortega
- Sarah Ramos Luna

## Project Overview
This starter bootstraps a simplified operating system simulator. On launch, the program prints a boot message, prompts for credentials, and validates them against a fixed administrator account. Later assignments will extend this foundation with additional OS components.

## File Structure
```
.
├─ README.md
├─ main.cpp          # Entry point: boot message + calls authenticateUser()
├─ auth.h            # Declaration of authenticateUser()
└─ auth.cpp          # Definition: prompts user and checks credentials
```

## Build Instructions

### Linux (g++)
1. Open a terminal in the project directory.
2. Compile:
   ```bash
   gcc -o main main.cpp auth.cpp
   ```
3. Run:
   ```bash
   ./main
   ```



## Running the Program
Launch the executable and enter the requested credentials.

- **Valid credentials**
  - Username: `admin`
  - Password: `password123`

## Expected Output

**Successful login**
```
Booting SimpleOS...
Username: admin
Password: password123
Access granted.
Login successful. Welcome!
```

**Failed login (example)**
```
Booting SimpleOS...
Username: admin
Password: wrongpass
Access denied.
Authentication failed.
```
