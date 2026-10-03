---
sidebar_position: 3
title: "CI/CD & Automated Labeling"
description: "Overview of GitHub Actions workflows, issue categorization, and automated PR triage."
---

# CI/CD & Automated Labeling

The PWSV repository uses **GitHub Actions** and **actions/labeler** to automatically categorize incoming Pull Requests according to the files modified.

---

## Labeler Workflow (`.github/workflows/label.yml`)

The labeler triggers on `pull_request_target`:

```yaml
name: Labeler
on: [pull_request_target]

jobs:
  label:
    runs-on: ubuntu-latest
    permissions:
      contents: read
      pull-requests: write
    steps:
    - uses: actions/labeler@8558fd74291d67161a8a78ce36a881fa63b766a9 # v5.0.0
      with:
        repo-token: "${{ secrets.GITHUB_TOKEN }}"
```

:::security Pull Request Target Safety
Because `pull_request_target` workflows run with write permissions in the context of the base repository, this workflow only inspects PR file paths for labeling and never checks out or executes untrusted code from pull request heads.
:::

---

## Category Mapping (`.github/labeler.yml`)

File changes are automatically labeled across multiple domains:
- **`build`**: `CMakeLists.txt`, `cmake/**`, `*.bat`
- **`ci`**: `.github/workflows/**`, `.gitignore`
- **`documentation`**: `*.md`, `docs/**`, `LICENSE`
- **`vst-plugin`**: `src/**`, `Source/**`
- **`websocket`**: `Source/WebSocket*`
- **`ui`**: `Source/*Editor*`
- **`audio`**: `Source/*Processor*`
- **`windows` / `macos` / `linux`**: Platform-specific changes
