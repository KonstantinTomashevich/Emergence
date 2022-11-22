# CelerityResourceConfig

This library provides support for game config resources. Config resource is an instance of particular type (for example,
UnitConfig or BuildingConfig), that is serialized to file in binary or YAML format. All configs of one type are
children of one folder that is called storage folder. Each config type has unique storage folder assigned to it.
Storage folder assignment is done through special config path mapping file (`ConfigPathMapping.bin` or 
`ConfigPathMapping.yaml`), which is stored in resources root folder.

`CelerityResourceConfig` provides API for loading configs by their type, which allows user to omit config storage
details in game code: these details are automatically loaded from config path mapping. Also, every config is loaded
into the game world as long term object and might be indexed in any way user needs it. 
