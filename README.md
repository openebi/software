# OpenEBI Software

OpenEBI is an open source project to measure electrical bioimpedance (EBI) using four-electrode technique. The hardware implements an AD5933-based spectrometer. Both magnitude and phase can be measured at the frequency range from few kilohertz up to 100 kHz. This is the GitHub repository of OpenEBI software. The hardware is located at https://github.com/openebi/hardware.

## Usage

Step 1: Install build tools and other essential support software

- If you are unsure of which development kit to use, install WinAVR.

Step 2: Fetch the source code and build

- Download OpenEBI source code or use git to clone the repository.
- Change dir to the directory including those source codes.
- Command `make`.
- Upload/Program/Flash the `main.hex` into the OpenEBI board via ISP bus.

## License

MIT License, see LICENSE.txt.
