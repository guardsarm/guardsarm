# Coverity scan tool

This directory contains a helper script and a Dockerfile for running [Coverity static analysis](https://scan.coverity.com/) on the GuardSarm codebase. It allows executing the analysis both locally and in CI environments using the same containerized setup.

## Files

- `coverity.sh`: main helper script for downloading the image, running the analysis, and uploading results.
- `Dockerfile`: defines the image used to compile GuardSarm with Coverity analysis enabled.

## Usage

```bash
./coverity.sh [--build-image] [--build] [--upload] [--clean] [--jobs N]
```

If run without arguments, the script will **compile** the project with Coverity and **upload** the results.

### Options

* `--build-image`
  Download the Coverity analysis tool and build the Docker image.
  The script exits immediately after this step.
  **Requires** `TOKEN` to be set.

* `--build`
  Compile the project using the Coverity Docker image and generate the output in `guardsarm.tgz`.
* `--upload`
  Upload `guardsarm.tgz` to Coverity Scan.
  **Requires** `TOKEN` to be set.
  Fails if the tarball does not exist.
* `--clean`
  Remove generated files (`cov-int/` directory and `guardsarm.tgz` tarball).
* `--jobs N`
  Set the number of parallel jobs used during compilation (default: system `nproc`).
* `--help`
  Show usage information.

### Environment variables

* `PROJECT`
  Coverity project name. Must be either `guardsarm` or `ossec-guardsarm`.
  Default: `guardsarm`.
  Allowed: `guardsarm` and `ossec-guardsarm`.
* `TOKEN`
  Coverity project token.
  Required for `--build-image` and `--upload`.
* `EMAIL`
  Email address associated with the Coverity account.
  Default: `devel@guardsarmsiem.com`.

### Build output

* The analysis results are generated in the `cov-int` directory under the project root.
* A compressed tarball `guardsarm.tgz` is created in the project root and uploaded to Coverity.

### Version and description

The uploaded analysis includes metadata extracted from the project:

* `VERSION` is taken from `VERSION.json` using `.version + "-" + .stage`.
* `DESCRIPTION` is set to: `Version $VERSION - Git ref <branch>`.

## Example: full workflow

```bash
# Build image (only needed once or when dependencies change)
COVERITY_TOKEN=your_token ./coverity.sh --build-image

# Run analysis and upload result
COVERITY_TOKEN=your_token ./coverity.sh

# Just upload the build to the ossec-guardsarm project
COVERITY_TOKEN=your_token PROJECT=ossec-guardsarm ./coverity.sh --upload

# Clean generated files after analysis
./coverity.sh --clean
```

## Notes

You can run this same scan from GitHub Actions, in the [4_codeanalysis_coverity](https://github.com/guardsarm/guardsarm/actions/workflows/4_codeanalysis_coverity.yml) workflow.
