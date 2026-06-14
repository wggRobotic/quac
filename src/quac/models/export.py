from ultralytics import YOLO
import sys
from pathlib import Path

if len(sys.argv) < 2:
    print("Usage: export.py <file path>")
    sys.exit()

model = YOLO(sys.argv[1])
imgsz = None

if hasattr(model.model, "args") and isinstance(model.model.args, dict):
    imgsz = model.model.args.get("imgsz")

if imgsz is None and hasattr(model, "overrides"):
    imgsz = model.overrides.get("imgsz")

if imgsz is None:
    print("Error: could not determine training image size")
    sys.exit()

print(f"Exporting with imgsz={imgsz}")

model.export(
    format="onnx",
    opset=18,
    imgsz=imgsz,
    dynamic=False,
    simplify=True
)

print("Export complete.")