# 5. Cooler Module Plan

The cooler purchase is deferred. Target an inexpensive 50–60 quart hard cooler, preferably with:

- Drain plug
- Flat sidewalls suitable for 3-inch fittings
- Hinged lid
- Enough width for a serpentine air path
- Wheels if the final assembly will be heavy

Premium ice retention is not required.

## Proposed airflow

```text
Fog inlet
   |
   v
+-----------------------------------+
| Upper inlet plenum                |
|  [frozen bottles / ice packs]     |
|-------------baffle----------------|
|             airflow down          |
|  [frozen bottles / ice packs]     |
|-------------baffle----------------|
| lower collection / condensate     |
+------------------------------+----+
                               |
                         outlet probe
                               |
                         external blower
                               |
                         diffuser hose
```

## Mechanical principles

- Place inlet and outlet at opposite ends.
- Force at least two direction changes.
- Avoid a straight line from inlet to outlet.
- Keep the outlet above the condensate floor.
- Put the blower outside the cooler for serviceability.
- Install a droplet separator before the blower.
- Use frozen bottles or sealed ice packs for repeatable testing.
- Retain access to the cooler drain.
- Avoid all 16 feet of flexible duct unless routing requires it; long corrugated duct adds resistance and condensation sites.

## Sensor locations

- Cooler sensor: suspended in the moving air, not touching ice or the wall.
- Outlet sensor: centered in the outlet stream, upstream of the blower if protected from droplets.
- Ambient BME280: outside the cooler and electronics enclosure.

## Outlet diffuser

A future 3-inch PVC manifold can lower exit velocity. Start with a short single outlet for characterization, then add the manifold after measuring airflow and fog behavior.
