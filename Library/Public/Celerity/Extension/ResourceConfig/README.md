# CelerityResourceConfig

This library provides support for game config resources. Config resource is an instance of particular type (for example,
UnitConfig or BuildingConfig), that is serialized to file in binary or YAML format. `CelerityResourceConfig` provides 
API for loading configs by their type, which allows user to omit config storage details in game code: loading is done 
through ResourceProvider, therefore paths are resolved automatically. Also, every config is loaded  into the game world 
as long term object and might be indexed in any way user needs it. 
