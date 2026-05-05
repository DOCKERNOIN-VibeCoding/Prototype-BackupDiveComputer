Import("env")

import os
import json

project_dir = env.subst("$PROJECT_DIR")

scenario_rel = env.GetProjectOption("custom_scenario", "scenarios/current.json")

if os.path.isabs(scenario_rel):
    scenario_path = scenario_rel
else:
    scenario_path = os.path.join(project_dir, scenario_rel)

output_path = os.path.join(project_dir, "include", "generated_scenario.h")

if not os.path.exists(scenario_path):
    raise FileNotFoundError(f"Scenario file not found: {scenario_path}")

with open(scenario_path, "r", encoding="utf-8") as f:
    data = json.load(f)


def c_string(s):
    if s is None:
        s = ""
    s = str(s)
    return s.replace("\\", "\\\\").replace("\"", "\\\"")


# ------------------------------------------------------------
# Basic scenario information
# ------------------------------------------------------------

name = data.get("name", "Unnamed Scenario")
description = data.get("description", "")
loop = bool(data.get("loop", False))

initial = data.get("initial", {})
initial_temp = float(initial.get("tempC", 24.0))

time_info = data.get("time", {})
start_epoch = int(time_info.get("startEpoch", 1735689600))
tz_offset_min = int(time_info.get("tzOffsetMin", 540))

points = data.get("points", [])

if len(points) < 2:
    raise ValueError("Scenario must have at least 2 points.")

normalized_points = []

for p in points:
    t_sec = float(p.get("t", 0))
    depth = float(p.get("depth", 0))
    temp = float(p.get("temp", initial_temp))

    if t_sec < 0:
        raise ValueError("Scenario point time cannot be negative.")

    if depth < 0:
        raise ValueError("Scenario depth cannot be negative.")

    normalized_points.append({
        "t_ms": int(round(t_sec * 1000.0)),
        "depth": depth,
        "temp": temp
    })

normalized_points.sort(key=lambda x: x["t_ms"])

for i in range(1, len(normalized_points)):
    if normalized_points[i]["t_ms"] <= normalized_points[i - 1]["t_ms"]:
        raise ValueError("Scenario point times must be strictly increasing.")


# ------------------------------------------------------------
# Preloaded virtual previous dive log
# ------------------------------------------------------------

preload = data.get("preload", {})
preload_enabled = bool(preload.get("enabled", False))

last_dive = preload.get("lastDive", {})

preload_dive_count = int(preload.get("diveCount", 0))
preload_last_duration_sec = int(last_dive.get("durationSec", 0))
preload_last_max_depth_m = float(last_dive.get("maxDepthM", 0.0))
preload_last_min_temp_c = float(last_dive.get("minTempC", initial_temp))

preload_last_start_epoch = int(last_dive.get("startEpoch", 0))
preload_last_duration_sec = int(last_dive.get("durationSec", 0))
preload_last_max_depth_m = float(last_dive.get("maxDepthM", 0.0))
preload_last_min_temp_c = float(last_dive.get("minTempC", initial_temp))
preload_last_no_fly_minutes_at_end = int(last_dive.get("noFlyMinutesAtEnd", 0))

# Backward compatibility for old scenario format
preload_surface_interval_sec = int(preload.get("surfaceIntervalSec", 0))
preload_no_fly_remain_sec = int(preload.get("noFlyRemainSec", 0))

if preload_last_start_epoch > 0 and preload_last_duration_sec > 0:
    preload_last_end_epoch = preload_last_start_epoch + preload_last_duration_sec
else:
    # Old format fallback:
    # current scenario start epoch - surface interval = last dive end epoch
    if preload_surface_interval_sec > 0 and preload_last_duration_sec > 0:
        preload_last_end_epoch = start_epoch - preload_surface_interval_sec
        preload_last_start_epoch = preload_last_end_epoch - preload_last_duration_sec
    else:
        preload_last_end_epoch = 0
        preload_last_start_epoch = 0

if preload_last_no_fly_minutes_at_end > 0 and preload_last_end_epoch > 0:
    preload_no_fly_end_epoch = preload_last_end_epoch + preload_last_no_fly_minutes_at_end * 60
else:
    # Old format fallback:
    # current scenario start epoch + no-fly remain
    if preload_no_fly_remain_sec > 0:
        preload_no_fly_end_epoch = start_epoch + preload_no_fly_remain_sec
    else:
        preload_no_fly_end_epoch = 0

gps = preload.get("gps", {})
preload_gps_valid = bool(gps.get("valid", False))
preload_gps_lat = float(gps.get("lat", 0.0))
preload_gps_lon = float(gps.get("lon", 0.0))
preload_gps_place = str(gps.get("place", ""))


# ------------------------------------------------------------
# Generate include/generated_scenario.h
# ------------------------------------------------------------

lines = []

lines.append("#pragma once")
lines.append("")
lines.append("#include <Arduino.h>")
lines.append("")
lines.append("// Auto-generated file. Do not edit manually.")
lines.append("// Generated from: {}".format(c_string(scenario_rel)))
lines.append("")
lines.append("struct ScenarioPoint {")
lines.append("  uint32_t tMs;")
lines.append("  float depthM;")
lines.append("  float tempC;")
lines.append("};")
lines.append("")
lines.append(f"static constexpr const char* SCENARIO_NAME = \"{c_string(name)}\";")
lines.append(f"static constexpr const char* SCENARIO_DESCRIPTION = \"{c_string(description)}\";")
lines.append(f"static constexpr bool SCENARIO_LOOP = {'true' if loop else 'false'};")
lines.append("")
lines.append(f"static constexpr float SCENARIO_INITIAL_TEMP_C = {initial_temp:.2f}f;")
lines.append("")
lines.append(f"static constexpr uint32_t SCENARIO_START_EPOCH = {start_epoch}UL;")
lines.append(f"static constexpr int16_t SCENARIO_TZ_OFFSET_MIN = {tz_offset_min};")
lines.append("")
lines.append("// ------------------------------------------------------------")
lines.append("// Preloaded virtual previous dive log state")
lines.append("// ------------------------------------------------------------")
lines.append(f"static constexpr bool SCENARIO_PRELOAD_ENABLED = {'true' if preload_enabled else 'false'};")
lines.append(f"static constexpr uint16_t SCENARIO_PRELOAD_DIVE_COUNT = {preload_dive_count};")

lines.append(f"static constexpr uint32_t SCENARIO_PRELOAD_LAST_START_EPOCH = {preload_last_start_epoch}UL;")
lines.append(f"static constexpr uint32_t SCENARIO_PRELOAD_LAST_DURATION_SEC = {preload_last_duration_sec}UL;")
lines.append(f"static constexpr uint32_t SCENARIO_PRELOAD_LAST_END_EPOCH = {preload_last_end_epoch}UL;")
lines.append(f"static constexpr float SCENARIO_PRELOAD_LAST_MAX_DEPTH_M = {preload_last_max_depth_m:.2f}f;")
lines.append(f"static constexpr float SCENARIO_PRELOAD_LAST_MIN_TEMP_C = {preload_last_min_temp_c:.2f}f;")
lines.append(f"static constexpr uint32_t SCENARIO_PRELOAD_NO_FLY_END_EPOCH = {preload_no_fly_end_epoch}UL;")

lines.append(f"static constexpr uint32_t SCENARIO_PRELOAD_SURFACE_INTERVAL_SEC = {preload_surface_interval_sec}UL;")
lines.append(f"static constexpr uint32_t SCENARIO_PRELOAD_NO_FLY_REMAIN_SEC = {preload_no_fly_remain_sec}UL;")

lines.append(f"static constexpr bool SCENARIO_PRELOAD_GPS_VALID = {'true' if preload_gps_valid else 'false'};")
lines.append(f"static constexpr float SCENARIO_PRELOAD_GPS_LAT = {preload_gps_lat:.7f}f;")
lines.append(f"static constexpr float SCENARIO_PRELOAD_GPS_LON = {preload_gps_lon:.7f}f;")
lines.append(f"static constexpr const char* SCENARIO_PRELOAD_GPS_PLACE = \"{c_string(preload_gps_place)}\";")
lines.append("")

lines.append("static const ScenarioPoint SCENARIO_POINTS[] = {")


for p in normalized_points:
    lines.append(
        f"  {{ {p['t_ms']}UL, {p['depth']:.2f}f, {p['temp']:.2f}f }},"
    )
lines.append("};")
lines.append("")
lines.append("static constexpr size_t SCENARIO_POINT_COUNT =")
lines.append("  sizeof(SCENARIO_POINTS) / sizeof(SCENARIO_POINTS[0]);")
lines.append("")

with open(output_path, "w", encoding="utf-8") as f:
    f.write("\n".join(lines))

print("")
print("============================================================")
print(" Scenario generated")
print("------------------------------------------------------------")
print(f" File        : {scenario_rel}")
print(f" Name        : {name}")
print(f" Points      : {len(normalized_points)}")
print(f" Preload     : {'enabled' if preload_enabled else 'disabled'}")
if preload_enabled:
    print(f" Dive Count  : {preload_dive_count}")
    print(f" Last Max    : {preload_last_max_depth_m:.1f} m")
    print(f" Surface Int : {preload_surface_interval_sec} sec")
    print(f" No-Fly Rem  : {preload_no_fly_remain_sec} sec")
print(f" Output      : include/generated_scenario.h")
print("============================================================")
print("")
