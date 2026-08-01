# Local ESPHome Development

Stage: 15C

Use a local ESPHome virtual environment to validate and compile ESPHome
projects without Home Assistant Builder.

## Environment

Recommended virtual environment:

```powershell
<venv-path>
```

Python executable:

```powershell
<venv-path>\Scripts\python.exe
```

Verified versions:

```text
Python: 3.11.7
pip: 26.2
ESPHome: 2026.6.5
PlatformIO Core: 6.1.19
Git: 2.54.0.windows.1
```

Notes:

- On some Windows machines, `python` on PATH may be the Windows Store alias.
- Use the venv Python path explicitly for reproducible builds.

## Activate The Venv

For an interactive PowerShell session:

```powershell
<venv-path>\Scripts\Activate.ps1
```

Activation is optional. Automation and Codex sessions should prefer the exact
Python executable path:

```powershell
<venv-path>\Scripts\python.exe -m esphome
```

## Update ESPHome

```powershell
<venv-path>\Scripts\python.exe -m pip install --upgrade pip setuptools wheel
<venv-path>\Scripts\python.exe -m pip install --upgrade esphome
```

Check the installed version:

```powershell
<venv-path>\Scripts\python.exe -m esphome version
```

## Validate YAML

```powershell
<venv-path>\Scripts\python.exe -m esphome config path\to\device.yaml
```

Useful help commands:

```powershell
<venv-path>\Scripts\python.exe -m esphome config --help
<venv-path>\Scripts\python.exe -m esphome compile --help
<venv-path>\Scripts\python.exe -m esphome run --help
```

## Compile Locally

Compile without upload:

```powershell
<venv-path>\Scripts\python.exe -m esphome compile path\to\device.yaml
```

Generate C++ only, without compiling:

```powershell
<venv-path>\Scripts\python.exe -m esphome compile --only-generate path\to\device.yaml
```

Run/upload should only be used when explicitly requested:

```powershell
<venv-path>\Scripts\python.exe -m esphome run path\to\device.yaml
```

## Stage 15C Verification

Temporary project path used during local validation:

```text
<temporary-esphome-test-project>
```

Verified files:

- `test-default.yaml`: minimal YAML from Stage 15C. With ESPHome 2026.6.5,
  ESP32-S3 defaults to ESP-IDF.
- `test-arduino.yaml`: same minimal ESP32-S3 logger project with explicit
  Arduino framework.
- `test-espidf.yaml`: same minimal ESP32-S3 logger project with explicit
  ESP-IDF framework.

Results:

- `python -m esphome version`: OK, ESPHome 2026.6.5.
- `python -m esphome config --help`: OK.
- `python -m esphome compile --help`: OK.
- `python -m esphome run --help`: OK.
- ESP-IDF compilation: OK. `test-default.yaml` built successfully.
- Arduino compilation: OK. `test-arduino.yaml` built successfully.

During the first ESP-IDF setup, a cancelled build left a broken PlatformIO
`tool-cmake` package. The package was removed and PlatformIO reinstalled it
successfully on the next build.

## Future Codex Command

Automation should invoke ESPHome with:

```powershell
<venv-path>\Scripts\python.exe -m esphome
```
