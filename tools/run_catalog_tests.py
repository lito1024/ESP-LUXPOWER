#!/usr/bin/env python3
"""Stage 31: independent tests over the canonical catalog's update_class
classification (data/luxpower_entities.json), separate from the generator's
own build-time validate_catalog() so a regression is caught even if someone
bypasses the generator.
"""
from __future__ import annotations

import json
from pathlib import Path

CATALOG_PATH = Path(__file__).resolve().parents[1] / "data" / "luxpower_entities.json"

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
VALID_UPDATE_CLASSES = {"FAST", "NORMAL", "SLOW"}


def load_entities() -> list[dict]:
    catalog = json.loads(CATALOG_PATH.read_text(encoding="utf-8"))
    return catalog["entities"]


def test_every_entity_has_exactly_one_valid_update_class() -> None:
    entities = load_entities()
    assert len(entities) > 0
    for entity in entities:
        assert "update_class" in entity, f"{entity['key']} missing update_class"
        assert entity["update_class"] in VALID_UPDATE_CLASSES, f"{entity['key']}: {entity['update_class']!r}"


def test_exactly_the_required_eight_fast_entities_are_fast() -> None:
    entities = load_entities()
    fast_keys = {e["key"] for e in entities if e["update_class"] == "FAST"}
    assert fast_keys == REQUIRED_FAST_KEYS, f"got {sorted(fast_keys)}, want {sorted(REQUIRED_FAST_KEYS)}"


def test_all_remaining_entities_are_normal_or_slow() -> None:
    entities = load_entities()
    for entity in entities:
        if entity["key"] in REQUIRED_FAST_KEYS:
            continue
        assert entity["update_class"] in ("NORMAL", "SLOW"), entity["key"]


def test_no_duplicate_keys_or_field_ids() -> None:
    entities = load_entities()
    keys = [e["key"] for e in entities]
    field_ids = [e["protocol_field_id"] for e in entities]
    assert len(keys) == len(set(keys)), "duplicate key(s) in catalog"
    assert len(field_ids) == len(set(field_ids)), "duplicate protocol_field_id(s) in catalog"


def test_load_power_present_fast_and_reachable_via_block2() -> None:
    entities = load_entities()
    load_power = next((e for e in entities if e["key"] == "load_power"), None)
    assert load_power is not None, "load_power missing from entities[]"
    assert load_power["update_class"] == "FAST"
    assert load_power["address"] == 170
    assert load_power["area"] == "input"
    assert load_power["status"] == "PRODUCTION"
    # Must no longer be in candidates[] (promoted, not left as NOT_WIRED).
    catalog = json.loads(CATALOG_PATH.read_text(encoding="utf-8"))
    candidate_keys = {c["key"] for c in catalog.get("candidates", [])}
    assert "load_power" not in candidate_keys


def test_power_to_grid_and_power_from_grid_are_fast_telemetry_group() -> None:
    # Stage 31: moved from DIAGNOSTICS to FAST_TELEMETRY group so they batch
    # with the other FAST entities; FieldId (9003/9004) unchanged.
    entities = load_entities()
    by_key = {e["key"]: e for e in entities}
    assert by_key["power_to_grid"]["protocol_group"] == "FAST_TELEMETRY"
    assert by_key["power_to_grid"]["protocol_field_id"] == 9003
    assert by_key["power_from_grid"]["protocol_group"] == "FAST_TELEMETRY"
    assert by_key["power_from_grid"]["protocol_field_id"] == 9004


def main() -> int:
    tests = [
        test_every_entity_has_exactly_one_valid_update_class,
        test_exactly_the_required_eight_fast_entities_are_fast,
        test_all_remaining_entities_are_normal_or_slow,
        test_no_duplicate_keys_or_field_ids,
        test_load_power_present_fast_and_reachable_via_block2,
        test_power_to_grid_and_power_from_grid_are_fast_telemetry_group,
    ]
    for test in tests:
        test()
        print(f"{test.__name__}: OK")
    print(f"{len(tests)} catalog tests passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
