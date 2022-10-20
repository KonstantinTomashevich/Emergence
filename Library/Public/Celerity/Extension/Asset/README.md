# Celerity: Asset extension

Implements asset caching behaviour: tracks asset references from long term objects using reflection and creates
asset nodes on demand. These asset nodes can be then picked up by actual asset loaders. Node usage count is updated
automatically and unused nodes will be cleaned up if special option is enabled in AssetManagerSingleton.
