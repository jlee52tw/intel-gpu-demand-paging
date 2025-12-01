# GPT-OSS-20B with OpenVINO GenAI

This repository contains scripts to run GPT-OSS-20B using OpenVINO GenAI.

## Model Conversion

To convert the GPT-OSS-20B model to OpenVINO IR (INT4) using the latest nightly OpenVINO GenAI build, follow these steps:

1.  **Setup Environment**
    Initialize the OpenVINO environment variables.
    ```cmd
    C:\working\gpt-oss-20b-custome-operator\openvino_genai_windows_2026.0.0.0.dev20251127_x86_64\setupvars.bat
    ```

2.  **Activate Python Environment**
    Navigate to the GenAI directory and activate the virtual environment.
    ```cmd
    cd C:\working\gpt-oss-20b-custome-operator\openvino_genai_windows_2026.0.0.0.dev20251127_x86_64
    env\Scripts\activate
    ```

3.  **Install Dependencies**
    Install the required packages for model export.
    ```cmd
    cd samples
    pip install -r export-requirements.txt
    ```

4.  **Export Model**
    Use `optimum-cli` to export the model to OpenVINO IR format with INT4 quantization.
    ```cmd
    optimum-cli export openvino -m openai/gpt-oss-20b "./ov_models/gpt-oss-20b" --task text-generation-with-past --weight-format int4
    ```

## Running Inference

Use the `gpt.py` script to run inference with the converted model.

```bash
python gpt.py
```

*Note: Ensure the `ov_model_path` in `gpt.py` points to your converted model directory (e.g., `./ov_models/gpt-oss-20b`).*
