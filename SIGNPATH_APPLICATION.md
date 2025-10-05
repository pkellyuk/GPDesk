# SignPath Foundation Application Guide

This document contains all the information needed to apply for free code signing through SignPath Foundation.

## Application URL

https://signpath.org/apply

## Project Information

**Project Name:** GPDesk

**Project Description:**
GPDesk is a gamepad PC control system for Windows that allows users to control their desktop using an Xbox-compatible controller. It provides mouse control, keyboard input, browser navigation, and system shortcuts all through gamepad input - perfect for couch gaming setups and accessibility needs.

**Repository URL:** https://github.com/pkellyuk/GPDesk

**License:** MIT License

**Project Category:** Desktop Utility / Accessibility Tool

**Primary Language:** C

**Target Platform:** Windows 10/11 (x64)

## Why We Need Code Signing

Currently, our executable triggers false positive warnings from Windows Defender (Trojan:Win32/Wacatac.C!ml) and browser download warnings because:

1. The application uses `SendInput()` Windows API for gamepad-to-keyboard/mouse translation
2. It requests administrator privileges to work with elevated applications like Task Manager
3. The executable is unsigned

These false positives create a significant barrier for users wanting to download and use GPDesk, despite the code being 100% open source and auditable.

Code signing will:
- Eliminate false positive virus warnings
- Build user trust through verified publisher identity
- Make the software more accessible to non-technical users
- Demonstrate commitment to security best practices

## Build Process

**Build System:** GitHub Actions (see `.github/workflows/build.yml`)

**Build Steps:**
1. Checkout code from repository
2. Setup MinGW-w64 compiler via msys2
3. Compile resource file with `windres`
4. Compile C source files with `gcc`
5. Link with Windows libraries
6. Output: `build/bin/gpdesk.exe`

**Artifact for Signing:** `build/bin/gpdesk.exe` (uploaded as GitHub artifact)

**Build Trigger:** Push to `main` branch or version tags (`v*`)

## SignPath Integration

Once approved, we will:

1. Add SignPath API token to GitHub repository secrets
2. Uncomment the SignPath submission step in `.github/workflows/build.yml`
3. Configure signing policy in SignPath dashboard
4. All releases will be automatically signed during CI/CD

## Project Maintainer

**GitHub Username:** pkellyuk
**Role:** Primary developer and maintainer

## Additional Information

- **First Release:** January 2025
- **Current Version:** v1.0.0
- **Active Development:** Yes, ongoing improvements
- **Community:** Open to contributions via GitHub pull requests
- **Documentation:** Full README with build instructions, usage guide, and architecture overview

## Technical Details

- **Compiler:** GCC (MinGW-w64)
- **Windows APIs Used:** XInput, SendInput, Shell API, Win32 windowing
- **Admin Privileges Required:** Yes (for system-wide input injection)
- **Network Activity:** None
- **External Dependencies:** None (only Windows system libraries)

## Contact

For any questions regarding this application:
- **GitHub Issues:** https://github.com/pkellyuk/GPDesk/issues
- **Repository Owner:** @pkellyuk

---

## Checklist for Application

Before submitting to SignPath Foundation, ensure:

- [ ] Repository is public on GitHub
- [ ] LICENSE file exists (MIT)
- [ ] README is comprehensive and up-to-date
- [ ] GitHub Actions workflow builds successfully
- [ ] Project follows open source best practices
- [ ] No proprietary or closed-source components
- [ ] Clear project description and purpose

## After Approval

Once SignPath Foundation approves the application:

1. They will provide:
   - Organization ID
   - API Token
   - Project slug

2. Add these as GitHub repository secrets:
   ```
   SIGNPATH_API_TOKEN=<provided-token>
   SIGNPATH_ORGANIZATION_ID=<provided-id>
   ```

3. Uncomment SignPath step in `.github/workflows/build.yml`

4. Next release will be automatically signed!

## Application Submission

To apply, visit: https://signpath.org/apply

Fill out the form with the information provided in this document. The review process typically takes a few business days.
