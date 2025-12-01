from pathlib import Path
import openvino_genai as ov_genai

# ------------------------------------------------------------------------------
# Streamer: prints tokens as they are generated
# ------------------------------------------------------------------------------
def console_streamer(subword: str) -> ov_genai.StreamingStatus:
    print(subword, end="", flush=True)
    return ov_genai.StreamingStatus.RUNNING


# ------------------------------------------------------------------------------
# Build long prompts of different approximate lengths
# ------------------------------------------------------------------------------
# This base chunk is a technical paragraph. Repeating it increases context length.
base_chunk = (
    "Modern AI runtime systems must coordinate CPU, GPU, and NPU execution while "
    "respecting power, thermal, and memory constraints. The runtime tracks key "
    "metrics such as TTFT (time to first token), TPOT (time per output token), "
    "and overall token throughput. To reach high efficiency, the system must map "
    "prefill and decode phases onto the right device, maintain KV-cache locality "
    "across tokens, and minimize page faults when using unified shared memory. "
    "In practice, this means balancing arithmetic intensity, memory bandwidth, "
    "and latency, while also reacting to changes in workload, user interaction "
    "patterns, and operating system power policies.\n\n"
)

def build_long_prompt(label: str, repeats: int) -> str:
    """
    Build a prompt with a long 'context' plus a final instruction.
    The repeats parameter controls approximate length (more repeats => more tokens).
    """
    header = (
        "You are a technical writer specializing in AI runtime systems. "
        "Read the following long context carefully. "
        "After the context, you will be asked to write a concise answer.\n\n"
        "IMPORTANT:\n"
        "- Do NOT describe your reasoning or planning.\n"
        "- Do NOT include words like 'analysis' or meta commentary.\n"
        "- Reply only with the final answer content.\n\n"
        f"### Context for scenario {label} ###\n\n"
    )

    body = base_chunk * repeats

    footer = (
        "### Task ###\n"
        "Using the above context, write a concise technical summary (one or two "
        "paragraphs) about how AI runtimes manage device scheduling, KV-cache "
        "memory, and latency metrics such as TTFT and TPOT. Reply directly with "
        "the summary only.\n"
    )

    return header + body + footer


# Short “base” prompt similar to what you used originally
base_prompt = (
    "Explain how OpenVINO optimizes AI inference on edge devices. "
    "Specifically discuss how it handles scheduling across CPU, GPU, and NPU "
    "on modern hybrid architectures such as Intel Lunar Lake or Panther Lake systems. "
    "Describe how memory-bound workloads benefit from model compression, operator "
    "fusion, and KV-cache locality. Focus on mechanisms like static vs dynamic "
    "shapes, unified shared memory, GPU page faults, and latency-critical "
    "execution paths. Provide a structured explanation. "
    "Do NOT include any meta commentary or analysis; answer directly."
)

# Approximate repeats for different “length tiers”
# You can bump these up/down after looking at `Input tokens:` in metrics.
repeats_for_len = {
    "len_1k": 8,   # ~1K-ish tokens
    "len_2k": 16,  # ~2K-ish
    "len_4k": 32,  # ~4K-ish
    "len_8k": 64,  # ~8K-ish (big!)
}

# ------------------------------------------------------------------------------
# OpenVINO GenAI pipeline setup
# ------------------------------------------------------------------------------
ov_model_path = Path("./model/gpt-oss-20b")
cache_dir = ov_model_path / "ov_cache"
cache_dir.mkdir(parents=True, exist_ok=True)

print("going to run openvino genai pipeline...")

pipe = ov_genai.LLMPipeline(
    str(ov_model_path),
    "GPU",
    CACHE_DIR=str(cache_dir),   # enable model cache
    ENABLE_MMAP=False,          # disable mmap
)

max_new_tokens = 128

# ------------------------------------------------------------------------------
# Prepare all prompts
# ------------------------------------------------------------------------------
prompts = []

# Base ~200-token prompt
prompts.append(("base_200", base_prompt))

# Long-context prompts of increasing length
for label, reps in repeats_for_len.items():
    prompts.append((label, build_long_prompt(label, reps)))


# ------------------------------------------------------------------------------
# Run all prompts sequentially with streaming + metrics
# ------------------------------------------------------------------------------
for name, prompt in prompts:
    print("=" * 80)
    print(f"PROMPT: {name}")
    print("-" * 80)
    print("Streaming output:\n")

    # List input so we get DecodedResults with perf_metrics
    result = pipe.generate(
        [prompt],
        max_new_tokens=max_new_tokens,
        streamer=console_streamer,
    )

    print("\n\n[END OF STREAM]\n")

    metrics = result.perf_metrics

    ttft       = metrics.get_ttft()
    tpot       = metrics.get_tpot()
    throughput = metrics.get_throughput()
    gen_dur    = metrics.get_generate_duration()
    infer_dur  = metrics.get_inference_duration()

    print("=== Perf metrics ===")
    print(f"TTFT (ms):             {ttft.mean:.3f}")
    print(f"TPOT (ms/token):       {tpot.mean:.3f}")
    print(f"Throughput (tok/s):    {throughput.mean:.3f}")
    print(f"Input tokens:          {metrics.get_num_input_tokens()}")
    print(f"Generated tokens:      {metrics.get_num_generated_tokens()}")
    print(f"Generate time (ms):    {gen_dur.mean:.3f}")
    print(f"Inference time (ms):   {infer_dur.mean:.3f}")
    print("\n")

del pipe
