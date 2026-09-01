# Entities

An `Entity` is an object in the simulation. Each entity can represent a physical object (e.g. aircraft, ship) or an non-physical object that performs some operation on the simulation world (e.g. data I/O).

## Entity Templates

An "entity template" is a prototype definition of an entity that can be instantiated into the world. For example, the world might contain two cars, car1 and car2, which both instantiate the same "car" entity template.
Entity template definitions are stored in `.entity.json` files and are located under the `/Entities` folder within an asset package.

Entities are defined by a list of components. Each component can have zero or more properties that can be configured in the `.entity.json` file.

{% for module_name, components in module_components.items() if module_name in ["_Core", "SkyboltBullet", "SkyboltJsbSim", "SkyboltPython"] -%}
{% if module_name.startswith("_") -%}
## {{ module_name[1:] }} Components
{% else -%}
## `{{ module_name }}` Plugin Components
{% endif -%}

{% for component_name, component in components.items() %}

### `{{ component_name }}`
{% if "description" in component -%}
{{ component.description }}
{% endif -%}
{% if "json_parsing_name" in component -%}
*entity.json parsing name:* ```{{ component.json_parsing_name }}```
{% endif -%}
#### Construction Parameters
{% if "construction_parameters" in component and component["construction_parameters"] %}
| Property | Type | Default | Description |
| :--- | :--- | :--- | :--- |
{% for parameter_name, parameter in component["construction_parameters"].items() -%}
| `{{ parameter_name }}` | `{{ parameter.type }}` | {{ parameter.default_value }} | {{ parameter.description }} |
{% endfor %}
{% else %}
None
{% endif %}

#### Properties
{% if "properties" in component and component["properties"] %}
| Property | Type |
| :--- | :--- |
{% for property_name, property in component["properties"].items() -%}
| `{{ property_name }}` | `{{ property.type }}` |
{% endfor -%}
{% else -%}
None
{% endif %}

{% endfor -%}
---
{% endfor %}