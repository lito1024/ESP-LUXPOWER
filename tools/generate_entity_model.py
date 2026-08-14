#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

HEADER = "AUTO-GENERATED - DO NOT EDIT MANUALLY"

# Stage 28 policy update: luxpower_entities.json is the canonical source of
# truth. Entities in entities[] are production by default; no dedicated
# manual hardware verification stage is required before an entity is
# generated, transported, displayed, and used. STATUS_ACTIVE entities are
# generated into the wire protocol; DEPRECATED entities are retired and
# stop being generated. candidates[] holds register mappings that are
# catalogued but not yet wired to the protocol at all (no protocol_field_id)
# -- documentation-only future work, never code-generated.
STATUS_PRODUCTION = "PRODUCTION"
STATUS_UNDER_REVIEW = "UNDER_REVIEW"
STATUS_KNOWN_ISSUE = "KNOWN_ISSUE"
STATUS_DEPRECATED = "DEPRECATED"
VALID_STATUSES = {STATUS_PRODUCTION, STATUS_UNDER_REVIEW, STATUS_KNOWN_ISSUE, STATUS_DEPRECATED}
ACTIVE_STATUSES = {STATUS_PRODUCTION, STATUS_UNDER_REVIEW, STATUS_KNOWN_ISSUE}

# Stage 39: non-LuxPower telemetry fields. These are intentionally NOT part
# of luxpower_entities.json and must not be generated into Gateway polling
# schedules, because Gateway is authoritative only for LuxPower source data.
# They do belong in the shared ESP-TELEMETRY field table and Display metadata.
SUPPLEMENTAL_PROTOCOL_ENTITIES = [
    {
        "key": "system_output_voltage",
        "name": "System Output Voltage",
        "unit": "V",
        "accuracy_decimals": 1,
        "status": STATUS_PRODUCTION,
        "protocol_field_id": 10001,
        "protocol_group": "FAST_TELEMETRY",
        "protocol_scale": 10,
        "protocol_type": "int32",
        "update_class": "FAST",
    },
    {
        "key": "system_output_current",
        "name": "System Output Current",
        "unit": "A",
        "accuracy_decimals": 1,
        "status": STATUS_PRODUCTION,
        "protocol_field_id": 10002,
        "protocol_group": "FAST_TELEMETRY",
        "protocol_scale": 100,
        "protocol_type": "int32",
        "update_class": "FAST",
    },
    {
        "key": "system_output_power",
        "name": "System Output Power",
        "unit": "W",
        "accuracy_decimals": 0,
        "status": STATUS_PRODUCTION,
        "protocol_field_id": 10003,
        "protocol_group": "FAST_TELEMETRY",
        "protocol_scale": 1,
        "protocol_type": "int32",
        "update_class": "FAST",
    },
    {
        "key": "system_output_energy",
        "name": "System Output Energy",
        "unit": "kWh",
        "accuracy_decimals": 2,
        "status": STATUS_PRODUCTION,
        "protocol_field_id": 10004,
        "protocol_group": "ENERGY_TOTALS",
        "protocol_scale": 100,
        "protocol_type": "int32",
        "update_class": "SLOW",
    },
    {
        "key": "system_output_frequency",
        "name": "System Output Frequency",
        "unit": "Hz",
        "accuracy_decimals": 1,
        "status": STATUS_PRODUCTION,
        "protocol_field_id": 10005,
        "protocol_group": "FAST_TELEMETRY",
        "protocol_scale": 10,
        "protocol_type": "int32",
        "update_class": "FAST",
    },
    {
        "key": "system_output_power_factor",
        "name": "System Output Power Factor",
        "unit": "",
        "accuracy_decimals": 2,
        "status": STATUS_PRODUCTION,
        "protocol_field_id": 10006,
        "protocol_group": "FAST_TELEMETRY",
        "protocol_scale": 100,
        "protocol_type": "int32",
        "update_class": "FAST",
    },
    {
        "key": "grid_input_voltage",
        "name": "Grid Input Voltage",
        "unit": "V",
        "accuracy_decimals": 1,
        "status": STATUS_PRODUCTION,
        "protocol_field_id": 10007,
        "protocol_group": "FAST_TELEMETRY",
        "protocol_scale": 10,
        "protocol_type": "int32",
        "update_class": "FAST",
    },
    {
        "key": "grid_input_current",
        "name": "Grid Input Current",
        "unit": "A",
        "accuracy_decimals": 1,
        "status": STATUS_PRODUCTION,
        "protocol_field_id": 10008,
        "protocol_group": "FAST_TELEMETRY",
        "protocol_scale": 100,
        "protocol_type": "int32",
        "update_class": "FAST",
    },
    {
        "key": "grid_input_power",
        "name": "Grid Input Power",
        "unit": "W",
        "accuracy_decimals": 0,
        "status": STATUS_PRODUCTION,
        "protocol_field_id": 10009,
        "protocol_group": "FAST_TELEMETRY",
        "protocol_scale": 1,
        "protocol_type": "int32",
        "update_class": "FAST",
    },
    {
        "key": "grid_input_energy",
        "name": "Grid Input Energy",
        "unit": "kWh",
        "accuracy_decimals": 2,
        "status": STATUS_PRODUCTION,
        "protocol_field_id": 10010,
        "protocol_group": "ENERGY_TOTALS",
        "protocol_scale": 100,
        "protocol_type": "int32",
        "update_class": "SLOW",
    },
    {
        "key": "grid_input_frequency",
        "name": "Grid Input Frequency",
        "unit": "Hz",
        "accuracy_decimals": 1,
        "status": STATUS_PRODUCTION,
        "protocol_field_id": 10011,
        "protocol_group": "FAST_TELEMETRY",
        "protocol_scale": 10,
        "protocol_type": "int32",
        "update_class": "FAST",
    },
    {
        "key": "grid_input_power_factor",
        "name": "Grid Input Power Factor",
        "unit": "",
        "accuracy_decimals": 2,
        "status": STATUS_PRODUCTION,
        "protocol_field_id": 10012,
        "protocol_group": "FAST_TELEMETRY",
        "protocol_scale": 100,
        "protocol_type": "int32",
        "update_class": "FAST",
    },
    {
        "key": "grid_raw_voltage",
        "name": "Grid Raw Voltage",
        "unit": "V",
        "accuracy_decimals": 1,
        "status": STATUS_PRODUCTION,
        "protocol_field_id": 10013,
        "protocol_group": "FAST_TELEMETRY",
        "protocol_scale": 10,
        "protocol_type": "int32",
        "update_class": "FAST",
    },
    {
        "key": "display_enabled",
        "name": "Display Enabled",
        "unit": "",
        "accuracy_decimals": 0,
        "status": STATUS_PRODUCTION,
        "protocol_field_id": 10014,
        "protocol_group": "FAST_TELEMETRY",
        "protocol_scale": 1,
        "protocol_type": "bool",
        "update_class": "FAST",
    },
    {
        "key": "garage_presence",
        "name": "Garage Presence",
        "unit": "",
        "accuracy_decimals": 0,
        "status": STATUS_PRODUCTION,
        "protocol_field_id": 11001,
        "protocol_group": "STATUS",
        "protocol_scale": 1,
        "protocol_type": "bool",
        "update_class": "FAST",
    },
]

# Stage 31: catalog-driven telemetry scheduling. Every production entity
# must have exactly one update_class, generated alongside FieldId so
# Gateway/Display never hand-maintain a duplicate class table.
UPDATE_CLASS_FAST = "FAST"
UPDATE_CLASS_NORMAL = "NORMAL"
UPDATE_CLASS_SLOW = "SLOW"
VALID_UPDATE_CLASSES = {UPDATE_CLASS_FAST, UPDATE_CLASS_NORMAL, UPDATE_CLASS_SLOW}
# The exact eight FAST entities required by Stage 31. No other entity may be
# FAST, and none of these may be assigned to another class or be missing.
REQUIRED_FAST_KEYS = {
    "pv1_power",
    "pv2_power",
    "load_power",
    "power_from_grid",
    "power_to_grid",
    "battery_charge_power",
    "battery_discharge_power",
    "battery_soc",
}

# Stage 31: the ONE configuration location for steady-state cadence per
# update_class (section 9). Gateway, docs, and tests all derive from these
# three numbers via the generated header -- never hand-duplicated.
FAST_INTERVAL_MS = 1000  # ~1s
NORMAL_INTERVAL_MS = 7500  # ~5-10s midpoint
SLOW_INTERVAL_MS = 45000  # ~30-60s midpoint


def pascal_case(key: str) -> str:
    return "".join(part.capitalize() for part in key.split("_"))


def enum_name(key: str) -> str:
    return "FieldId" + pascal_case(key)


def cpp_string(value: str | None) -> str:
    if value is None:
        return "nullptr"
    return '"' + value.replace("\\", "\\\\").replace('"', '\\"') + '"'


def load_catalog(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def active_entities(catalog: dict) -> list[dict]:
    """Entities that are generated into the wire protocol: everything except DEPRECATED."""
    return [entity for entity in catalog["entities"] if entity["status"] in ACTIVE_STATUSES]


def protocol_entities(catalog: dict) -> list[dict]:
    return active_entities(catalog) + SUPPLEMENTAL_PROTOCOL_ENTITIES


def validate_catalog(catalog: dict) -> list[str]:
    errors: list[str] = []
    entities = catalog.get("entities", [])
    candidates = catalog.get("candidates", [])
    keys = [entity.get("key") for entity in entities] + [item.get("key") for item in candidates]
    field_ids = [entity.get("protocol_field_id") for entity in entities]

    for label, values in (("key", keys), ("protocol_field_id", field_ids)):
        seen = set()
        for value in values:
            if value in seen:
                errors.append(f"duplicate {label}: {value}")
            seen.add(value)

    for item in candidates:
        if "protocol_field_id" in item:
            errors.append(
                f"{item.get('key')}: candidates[] entries must not carry protocol_field_id -- "
                "a catalog entry with wire fields is production telemetry and belongs in entities[]"
            )
        for required in ("key", "name", "status", "unit", "reason_excluded"):
            if required not in item:
                errors.append(f"candidate {item.get('key')} missing {required}")

    key_pattern = re.compile(r"^[a-z][a-z0-9_]*$")
    for entity in entities:
        key = entity.get("key", "")
        if not key_pattern.match(key):
            errors.append(f"invalid key: {key}")
        if entity.get("status") not in VALID_STATUSES:
            errors.append(f"{key}: invalid status {entity.get('status')!r}, must be one of {sorted(VALID_STATUSES)}")
        if entity.get("writable") is not False:
            errors.append(f"production entity must be read-only: {key}")
        for required in (
            "name",
            "category",
            "area",
            "raw_type",
            "unit",
            "accuracy_decimals",
            "cache_dependency",
            "protocol_field_id",
            "protocol_group",
            "protocol_scale",
            "protocol_type",
            "update_class",
        ):
            if required not in entity:
                errors.append(f"{key} missing {required}")

    # Stage 31: update_class validation, independent of the per-field loop
    # above so the FAST-set check considers the whole catalog at once.
    fast_keys_found: set[str] = set()
    for entity in entities:
        key = entity.get("key", "")
        update_class = entity.get("update_class")
        if update_class is None:
            continue  # already reported as a missing-field error above
        if update_class not in VALID_UPDATE_CLASSES:
            errors.append(f"{key}: invalid update_class {update_class!r}, must be one of {sorted(VALID_UPDATE_CLASSES)}")
            continue
        if update_class == UPDATE_CLASS_FAST:
            fast_keys_found.add(key)

    unexpected_fast = fast_keys_found - REQUIRED_FAST_KEYS
    if unexpected_fast:
        errors.append(
            "entities assigned FAST but not in the required eight: "
            f"{sorted(unexpected_fast)} -- FAST is reserved for {sorted(REQUIRED_FAST_KEYS)}"
        )
    missing_fast = REQUIRED_FAST_KEYS - fast_keys_found
    if missing_fast:
        entity_keys = {entity.get("key") for entity in entities}
        for missing_key in sorted(missing_fast):
            if missing_key not in entity_keys:
                errors.append(f"required FAST entity {missing_key!r} is missing from entities[] entirely")
            else:
                errors.append(f"required FAST entity {missing_key!r} exists but is not assigned update_class FAST")

    return errors


def group_entities(entities: list[dict]) -> dict[str, list[dict]]:
    grouped: dict[str, list[dict]] = {}
    for entity in entities:
        grouped.setdefault(entity["category"], []).append(entity)
    for values in grouped.values():
        values.sort(key=lambda item: item["protocol_field_id"])
    return dict(sorted(grouped.items()))


def markdown_table(rows: list[list[object]], headers: list[str]) -> str:
    lines = [
        "| " + " | ".join(headers) + " |",
        "| " + " | ".join("---" for _ in headers) + " |",
    ]
    for row in rows:
        lines.append("| " + " | ".join("" if value is None else str(value) for value in row) + " |")
    return "\n".join(lines)


def render_verified_catalog(catalog: dict) -> str:
    entities = active_entities(catalog)
    lines = [f"<!-- {HEADER} -->", "# Entity Catalog", ""]
    lines.append("Source catalog: `data/luxpower_entities.json`.")
    lines.append("")
    lines.append(
        "Stage 28 policy: `luxpower_entities.json` is the canonical source of truth. "
        "Entities in `entities[]` are production by default (status `PRODUCTION`, "
        "`UNDER_REVIEW`, or `KNOWN_ISSUE` -- all three are generated/transported/displayed). "
        "`DEPRECATED` entities are retired and no longer generated. A real discrepancy "
        "downgrades only the affected entity to `UNDER_REVIEW`, never unrelated entities."
    )
    lines.append("")

    for category, category_entities in group_entities(entities).items():
        lines.extend([f"## {category}", ""])
        rows = []
        for entity in category_entities:
            source = entity["area"]
            if entity["area"] in {"input", "holding"}:
                source += f"[{entity['address']}]"
                if entity.get("bit") is not None:
                    source += f" bit {entity['bit']}"
            rows.append(
                [
                    entity["key"],
                    entity["name"],
                    source,
                    entity["raw_type"],
                    entity["scale"],
                    entity["unit"],
                    entity["status"],
                    entity["update_class"],
                    entity["protocol_group"],
                    entity["notes"],
                ]
            )
        lines.append(
            markdown_table(
                rows,
                [
                    "Key",
                    "Name",
                    "Source register",
                    "Type",
                    "Scale",
                    "Unit",
                    "Status",
                    "Update class",
                    "Protocol group",
                    "Notes",
                ],
            )
        )
        lines.append("")

    non_production = [entity for entity in entities if entity["status"] != STATUS_PRODUCTION]
    lines.extend(["## Entities Under Review Or With Known Issues", ""])
    if non_production:
        lines.append(
            markdown_table(
                [[entity["key"], entity["status"], entity["notes"]] for entity in non_production],
                ["Key", "Status", "Notes"],
            )
        )
    else:
        lines.append("None. All production entities are currently in normal `PRODUCTION` status.")

    lines.extend(["", "## Catalogued But Not Wired", ""])
    lines.append(
        "Register mappings that are catalogued in `candidates[]` but do not carry a "
        "`protocol_field_id` -- they are not part of the wire protocol at all yet, and "
        "this is a wiring/transport gap, not a verification gap."
    )
    lines.append("")
    lines.append(
        markdown_table(
            [
                [
                    item["key"],
                    item["name"],
                    item["status"],
                    item.get("area", ""),
                    item.get("address", item.get("addresses", "")),
                    item.get("unit", ""),
                    item["reason_excluded"],
                ]
                for item in catalog.get("candidates", [])
            ],
            ["Key", "Name", "Status", "Area", "Address", "Unit", "Reason not wired"],
        )
    )

    lines.extend(["", "## Potential Future Controls Not Included", ""])
    lines.append(
        markdown_table(
            [
                [
                    item["key"],
                    item["semantic_intent"],
                    item["source"],
                    item["verification_status"],
                    item["safety_constraints"],
                ]
                for item in catalog.get("future_controls", [])
            ],
            ["Key", "Semantic intent", "Source", "Status", "Safety constraints"],
        )
    )
    lines.extend(["", "## Duplicates / Aliases", "", "None identified in the catalog."])
    return "\n".join(lines) + "\n"


def render_future_controls(catalog: dict) -> str:
    lines = [f"<!-- {HEADER} -->", "# Future Control Entities", ""]
    lines.append("Controls are documentation-only and are not exposed to ESPHome or UART.")
    lines.append("")
    lines.append(
        markdown_table(
            [
                [
                    item["key"],
                    item["semantic_intent"],
                    item["source"],
                    item["required_range_or_enum"],
                    item["verification_status"],
                    item["safety_constraints"],
                ]
                for item in catalog.get("future_controls", [])
            ],
            ["Key", "Semantic intent", "Source", "Range / enum", "Status", "Safety constraints"],
        )
    )
    return "\n".join(lines) + "\n"


def render_protocol_fields_h(catalog: dict) -> str:
    entities = sorted(protocol_entities(catalog), key=lambda item: item["protocol_field_id"])
    lines = [
        f"// {HEADER}",
        "#ifndef ESP_TELEMETRY_FIELDS_H",
        "#define ESP_TELEMETRY_FIELDS_H",
        "",
        "#include <stdint.h>",
        "",
        "namespace ESPTelemetry",
        "{",
        "",
        "enum FieldId : uint16_t",
        "{",
    ]
    for entity in entities:
        comment = "" if entity["status"] == STATUS_PRODUCTION else f"  // {entity['status']}"
        lines.append(f"    {enum_name(entity['key'])} = {entity['protocol_field_id']},{comment}")
    lines.extend(
        [
            "};",
            "",
            "enum class TelemetryGroup : uint8_t",
            "{",
            "    FastTelemetry = 1,",
            "    EnergyTotals = 2,",
            "    Status = 3,",
            "    Diagnostics = 4,",
            "};",
            "",
            "// Stage 31: catalog-driven scheduling class. Generated so Gateway and",
            "// Display never hand-maintain a duplicate FAST/NORMAL/SLOW table.",
            "enum class UpdateClass : uint8_t",
            "{",
            "    Fast = 1,",
            "    Normal = 2,",
            "    Slow = 3,",
            "};",
            "",
            "struct FieldDescriptor",
            "{",
            "    uint16_t id;",
            "    const char *key;",
            "    const char *name;",
            "    const char *unit;",
            "    int32_t scale;",
            "    TelemetryGroup group;",
            "    UpdateClass updateClass;",
            "};",
            "",
            "static constexpr FieldDescriptor kFieldDescriptors[] = {",
        ]
    )
    group_map = {
        "FAST_TELEMETRY": "TelemetryGroup::FastTelemetry",
        "ENERGY_TOTALS": "TelemetryGroup::EnergyTotals",
        "STATUS": "TelemetryGroup::Status",
        "DIAGNOSTICS": "TelemetryGroup::Diagnostics",
    }
    update_class_map = {
        UPDATE_CLASS_FAST: "UpdateClass::Fast",
        UPDATE_CLASS_NORMAL: "UpdateClass::Normal",
        UPDATE_CLASS_SLOW: "UpdateClass::Slow",
    }
    for entity in entities:
        lines.append(
            "    {"
            f"{enum_name(entity['key'])}, {cpp_string(entity['key'])}, {cpp_string(entity['name'])}, "
            f"{cpp_string(entity['unit'])}, {entity['protocol_scale']}, {group_map[entity['protocol_group']]}, "
            f"{update_class_map[entity['update_class']]}"
            "},"
        )
    lines.extend(
        [
            "};",
            "",
            "static constexpr uint8_t kGeneratedFieldCount = sizeof(kFieldDescriptors) / sizeof(kFieldDescriptors[0]);",
            "",
            "} // namespace ESPTelemetry",
            "",
            "#endif",
            "",
        ]
    )
    return "\n".join(lines)


def render_gateway_metadata_h(catalog: dict) -> str:
    entities = sorted(active_entities(catalog), key=lambda item: item["protocol_field_id"])
    lines = [
        f"// {HEADER}",
        "#ifndef LUXPOWER_TELEMETRY_GENERATED_H",
        "#define LUXPOWER_TELEMETRY_GENERATED_H",
        "",
        "#include <stdint.h>",
        "#include \"ESPTelemetryFields.h\"",
        "",
        "// Stage 31: catalog-driven scheduling. This is the one configuration",
        "// location for FAST/NORMAL/SLOW cadence -- do not hand-duplicate these",
        "// intervals elsewhere. Gateway groups fields by kFieldDescriptors[].",
        "// updateClass at startup instead of maintaining its own entity lists.",
        f"static constexpr uint32_t kFastUpdateIntervalMs = {FAST_INTERVAL_MS};",
        f"static constexpr uint32_t kNormalUpdateIntervalMs = {NORMAL_INTERVAL_MS};",
        f"static constexpr uint32_t kSlowUpdateIntervalMs = {SLOW_INTERVAL_MS};",
        "",
        "struct LuxPowerTelemetryFieldSchedule",
        "{",
        "    uint16_t fieldId;",
        "    uint32_t intervalMs;",
        "};",
        "",
        "static constexpr LuxPowerTelemetryFieldSchedule kLuxPowerTelemetryFieldSchedule[] = {",
    ]
    update_class_intervals = {
        UPDATE_CLASS_FAST: "kFastUpdateIntervalMs",
        UPDATE_CLASS_NORMAL: "kNormalUpdateIntervalMs",
        UPDATE_CLASS_SLOW: "kSlowUpdateIntervalMs",
    }
    for entity in entities:
        lines.append(
            f"    {{ESPTelemetry::{enum_name(entity['key'])}, {update_class_intervals[entity['update_class']]} }},"
        )
    lines.extend(
        [
            "};",
            "",
            "#endif",
            "",
        ]
    )
    return "\n".join(lines)


def render_display_metadata_h(catalog: dict) -> str:
    entities = sorted(protocol_entities(catalog), key=lambda item: item["protocol_field_id"])
    lines = [
        f"// {HEADER}",
        "#pragma once",
        "",
        "#include <stdint.h>",
        "",
        "namespace esphome {",
        "namespace display_protocol_uart {",
        "",
        "// update_class is informational here -- Display accepts fields",
        "// regardless of class (section 13); it is not used to gate publish.",
        "struct DisplayProtocolEntityMetadata {",
        "  uint16_t field_id;",
        "  const char *key;",
        "  const char *name;",
        "  const char *unit;",
        "  uint8_t accuracy_decimals;",
        "  const char *update_class;",
        "};",
        "",
        "static constexpr DisplayProtocolEntityMetadata kDisplayProtocolEntityMetadata[] = {",
    ]
    for entity in entities:
        lines.append(
            f"  {{{entity['protocol_field_id']}, {cpp_string(entity['key'])}, {cpp_string(entity['name'])}, "
            f"{cpp_string(entity['unit'])}, {entity['accuracy_decimals']}, {cpp_string(entity['update_class'])}}},"
        )
    lines.extend(["};", "", "}  // namespace display_protocol_uart", "}  // namespace esphome", ""])
    return "\n".join(lines)


def render_protocol_v2(catalog: dict) -> str:
    entities = sorted(protocol_entities(catalog), key=lambda item: item["protocol_field_id"])
    lines = [f"<!-- {HEADER} -->", "# ESP Telemetry Protocol V2", ""]
    lines.append("Protocol v2 keeps the existing `SPDP` magic and CRC16 protection while allowing grouped semantic telemetry fields.")
    lines.append("")
    lines.extend(
        [
            "## Frame Header",
            "",
            markdown_table(
                [
                    ["0", "4", "magic", "`SPDP`"],
                    ["4", "1", "version", "`1` for legacy snapshot, `2` for v2"],
                    ["5", "1", "message_type", "semantic message type"],
                    ["6", "2", "payload_length", "little-endian bytes"],
                    ["8", "2", "sequence", "little-endian, independent per transmitted stream"],
                    ["10", "4", "gateway_timestamp_ms", "little-endian milliseconds"],
                    ["14", "N", "payload", "message-specific"],
                    ["14+N", "2", "crc16", "Modbus CRC16 over header + payload"],
                ],
                ["Offset", "Size", "Field", "Description"],
            ),
            "",
            "## Message Types",
            "",
            markdown_table(
                [
                    ["1", "V1_SNAPSHOT", "Existing fixed 48-byte frame, decoded for backward compatibility."],
                    ["16", "FAST_TELEMETRY", "Live PV and battery values, about every 1 second."],
                    ["17", "ENERGY_TOTALS", "Energy counters, about every 30 seconds."],
                    ["18", "STATUS", "Read-only status flags, about every 5 seconds."],
                    ["19", "DIAGNOSTICS", "Gateway/protocol diagnostics plus lower-cadence production fields."],
                    ["32", "COMMAND", "Controller -> Gateway typed command request."],
                    ["33", "COMMAND_RESULT", "Gateway -> Controller typed command result."],
                    ["34-63", "Future command range", "Reserved for future typed commands."],
                ],
                ["ID", "Name", "Purpose"],
            ),
            "",
            "## V2 Telemetry Payload",
            "",
            "No floats are sent on the wire. Values are signed 32-bit scaled integers.",
            "",
            markdown_table(
                [
                    ["0", "1", "device_id", "Source identity: `1` inverter 1, `2` inverter 2, `3` Controller/system, `4` Garage Sensor"],
                    ["1", "1", "group_id", "1 fast, 2 energy, 3 status, 4 diagnostics"],
                    ["2", "1", "field_count", "number of following field records"],
                    ["3", "2", "group_flags", "bit 0 = group values valid, bit 1 = cache valid"],
                    ["5", "6 * field_count", "records", "`field_id:u16` + `value:i32`, little-endian"],
                ],
                ["Offset", "Size", "Field", "Description"],
            ),
            "",
            "Unknown field IDs and unknown message types are skipped after CRC validation.",
            "",
            "## Field IDs",
            "",
            "Stage 28 policy: all fields listed here are production (status `PRODUCTION`, "
            "`UNDER_REVIEW`, or `KNOWN_ISSUE` -- see the Status column). There is no separate "
            "non-production field ID range any more; a catalog entity either has a "
            "`protocol_field_id` and is generated here, or it has none and stays "
            "documentation-only in `candidates[]`.",
            "",
        ]
    )
    lines.append(
        markdown_table(
            [
                [
                    entity["protocol_field_id"],
                    entity["key"],
                    entity["name"],
                    entity["protocol_group"],
                    entity["update_class"],
                    entity["protocol_type"],
                    entity["protocol_scale"],
                    entity["unit"],
                    entity["status"],
                ]
                for entity in entities
            ],
            ["Field ID", "Key", "Name", "Group", "Update class", "Wire type", "Wire scale", "Unit", "Status"],
        )
    )
    lines.extend(
        [
            "",
            "## V1 Compatibility",
            "",
            "The decoder accepts both version 1 snapshot frames and version 2 telemetry frames. Existing v1 receivers continue to work while the gateway is in `V1_AND_V2` mode.",
            "",
            "## Cadence And Bandwidth",
            "",
            "With the current catalog, typical v2 traffic is one FAST frame per second, one STATUS frame every 5 seconds, one ENERGY frame every 30 seconds, and one DIAGNOSTICS frame every 10 seconds.",
            "",
            "- FAST payload: 34 bytes, frame: 50 bytes.",
            "- STATUS payload: 10 bytes, frame: 26 bytes.",
            "- ENERGY payload: 10 bytes, frame: 26 bytes.",
            "- DIAGNOSTICS payload: 34 bytes, frame: 50 bytes.",
            "- Typical v2 bandwidth: about 60 bytes/s.",
            "- Migration `V1_AND_V2` with one v1 frame/s adds 48 bytes/s, total about 108 bytes/s.",
            "- 230400 baud is about 23040 bytes/s at 10 bits/byte, so migration traffic is below 0.5% of line capacity.",
            "",
        ]
    )
    return "\n".join(lines)


def write_or_check(path: Path, content: str, check: bool, changed: list[Path]) -> None:
    if path.exists() and path.read_text(encoding="utf-8") == content:
        return
    changed.append(path)
    if not check:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8", newline="\n")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true", help="fail if generated files are stale")
    parser.add_argument("--luxpower-root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--protocol-root", type=Path, default=Path(r"C:\\Projects\\ESP\\ESP-TELEMETRY"))
    parser.add_argument("--gateway-root", type=Path, default=Path(r"C:\Projects\ESP\luxpower-gateway"))
    parser.add_argument("--display-root", type=Path, default=Path(r"C:\Projects\ESP\sweet-power-display"))
    args = parser.parse_args()

    catalog_path = args.luxpower_root / "data" / "luxpower_entities.json"
    catalog = load_catalog(catalog_path)
    errors = validate_catalog(catalog)
    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1

    outputs = {
        args.luxpower_root / "docs" / "VerifiedEntityCatalog.md": render_verified_catalog(catalog),
        args.luxpower_root / "docs" / "FutureControlEntities.md": render_future_controls(catalog),
        args.protocol_root / "src" / "ESPTelemetryFields.h": render_protocol_fields_h(catalog),
        args.protocol_root / "docs" / "ProtocolV2.md": render_protocol_v2(catalog),
        args.gateway_root / "include" / "LuxPowerTelemetry.generated.h": render_gateway_metadata_h(catalog),
        args.display_root
        / "components"
        / "display_protocol_uart"
        / "display_protocol_entity_metadata.generated.h": render_display_metadata_h(catalog),
    }

    changed: list[Path] = []
    for path, content in outputs.items():
        write_or_check(path, content, args.check, changed)

    if args.check and changed:
        for path in changed:
            print(f"stale generated file: {path}", file=sys.stderr)
        return 1

    for path in changed:
        print(f"generated {path}")
    if not changed:
        print("generated files are up to date")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
