#!/usr/bin/env python3
"""Rebuild ps4_keys_default.h from PS4Auth/key.pem, serial.txt, and sig.bin."""

from pathlib import Path
import re
import subprocess

HERE = Path(__file__).resolve().parent
AUTH = HERE / "PS4Auth"


def parse_mpi(sections, label):
    rest = sections[label]
    first = rest.strip().split("\n", 1)[0].strip()
    if re.fullmatch(r"\d+(?:\s+\(0x[0-9a-fA-F]+\))?", first):
        return int(first.split()[0])
    return int(re.sub(r"[^0-9a-fA-F]", "", rest), 16)


def c_array(name, data, width=16):
    lines = [f"static const uint8_t {name}[{len(data)}] = {{"]
    for i in range(0, len(data), width):
        chunk = ", ".join(f"0x{b:02X}" for b in data[i : i + width])
        lines.append(f"    {chunk},")
    lines.append("};")
    return "\n".join(lines)


def main():
    text = subprocess.check_output(
        ["openssl", "rsa", "-in", str(AUTH / "key.pem"), "-text", "-noout"],
        text=True,
    )
    parts = re.split(r"\n(?=[A-Za-z][A-Za-z0-9 ]*:)", text)
    sections = {}
    for part in parts:
        if ":" not in part:
            continue
        label, rest = part.split(":", 1)
        sections[label.strip()] = rest

    n = parse_mpi(sections, "modulus")
    e = parse_mpi(sections, "publicExponent")
    p = parse_mpi(sections, "prime1")
    q = parse_mpi(sections, "prime2")

    serial_txt = (AUTH / "serial.txt").read_text().strip()
    signature = (AUTH / "sig.bin").read_bytes()
    if len(serial_txt) != 16 or len(signature) != 256:
        raise SystemExit("serial.txt must be 16 hex chars and sig.bin must be 256 bytes")

    serial = bytes.fromhex(serial_txt.rjust(32, "0"))
    out = HERE / "ps4_keys_default.h"
    out.write_text(
        "#ifndef BOARD_17PNT_PS4_KEYS_DEFAULT_H_\n"
        "#define BOARD_17PNT_PS4_KEYS_DEFAULT_H_\n\n"
        "#include <stdint.h>\n\n"
        "#define PS4_DEFAULT_KEYS_EMBEDDED 1\n\n"
        + c_array("kPs4DefaultSerial", serial)
        + "\n\n"
        + c_array("kPs4DefaultSignature", signature)
        + "\n\n"
        + c_array("kPs4DefaultRsaN", n.to_bytes(256, "little"))
        + "\n\n"
        + c_array("kPs4DefaultRsaE", e.to_bytes(4, "little"))
        + "\n\n"
        + c_array("kPs4DefaultRsaP", p.to_bytes(128, "little"))
        + "\n\n"
        + c_array("kPs4DefaultRsaQ", q.to_bytes(128, "little"))
        + "\n\n"
        "#endif\n"
    )
    print(f"wrote {out}")


if __name__ == "__main__":
    main()
