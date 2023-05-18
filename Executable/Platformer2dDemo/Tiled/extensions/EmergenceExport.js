let saveSpriteMaterialInstance = (rootFolder, instanceName, textureName) => {
    let instancesFolder = rootFolder + "/MaterialInstances/";
    let materialFile = new TextFile(instancesFolder + instanceName + ".material.instance.yaml", TextFile.WriteOnly);
    materialFile.writeLine("material: Sprite");
    materialFile.writeLine("uniforms:");
    materialFile.writeLine("  - name: colorTexture");
    materialFile.writeLine("    type: 3");
    materialFile.writeLine("    textureId: " + textureName);
    materialFile.commit();
};

let extractLayers = (layerContainer) => {
    let layers = [];
    layerContainer.layers.forEach(layer => {
        if (layer.isGroupLayer) {
            layers.concat(extractLayers(layer));
        } else {
            layers.push(layer);
        }
    });

    return layers;
};

let saveMapObject = (file, objectId, className, locationX, locationY, rotation, layerIndex) => {
    file.writeLine("  - component:");
    file.writeLine("      type: Transform2dComponent");
    file.writeLine("      content:");
    file.writeLine("        objectId: " + objectId);
    file.writeLine("        logicalLocalTransform:");
    file.writeLine("          translation:");
    file.writeLine("            x: " + locationX);
    file.writeLine("            y: " + locationY);
    file.writeLine("          rotation: " + rotation);

    file.writeLine("  - component:");
    file.writeLine("      type: LayerSetupComponent");
    file.writeLine("      content:");
    file.writeLine("        objectId: " + objectId);
    file.writeLine("        layer: " + layerIndex);

    file.writeLine("  - component:");
    file.writeLine("      type: PrototypeComponent")
    file.writeLine("      content:")
    file.writeLine("        objectId: " + objectId);
    file.writeLine("        descriptorId: " + className);
    file.writeLine("        requestImmediateFixedAssembly: 0");
    file.writeLine("        requestImmediateNormalAssembly: 0");
};

tiled.registerTilesetFormat(
    "EmergenceResourcesSet", {
        name: "Emergence Resources",
        extension: "stub",

        write: (tileset, fileName) => {
            if (!fileName.endsWith("/EmergenceResourcesRoot.stub")) {
                tiled.alert("Incorrect save name! EmergenceResourcesRoot.stub file expected!", "Export failed")
                tiled.log(fileName)
                return;
            }

            let columns = Math.trunc(tileset.imageWidth / (tileset.tileWidth + tileset.tileSpacing));
            let libraryDirectory = FileInfo.path(fileName) + "/Objects/" + tileset.name;
            File.makePath(libraryDirectory);
            tiled.log("Saving Emergence Assembly library to \"" + libraryDirectory + "\".");

            // Save material for atlas-based tilesets.
            if (tileset.image !== "") {
                saveSpriteMaterialInstance(
                    FileInfo.path(fileName),
                    FileInfo.baseName(tileset.image),
                    FileInfo.fileName(tileset.image));
            }

            let folderDependenciesFile = new BinaryFile(
                libraryDirectory + "/ObjectFolderDependencies.bin", BinaryFile.WriteOnly);
            folderDependenciesFile.commit();

            tileset.tiles.forEach(tile => {
                if (tile.className == null || tile.className === "") {
                    return;
                }

                let tileObjectFile = new TextFile(
                    libraryDirectory + "/" + tile.className + ".object.yaml", TextFile.WriteOnly);

                tileObjectFile.writeLine("parent: \"\"")
                tileObjectFile.writeLine("changelist:")

                tileObjectFile.writeLine("  - component:");
                tileObjectFile.writeLine("      type: Sprite2dComponent");
                tileObjectFile.writeLine("      content:");
                tileObjectFile.writeLine("        objectId: 0");
                tileObjectFile.writeLine("        spriteId: 0");

                var minX;
                var minY;
                var maxX;
                var maxY;

                if (tileset.image === "") {
                    tileObjectFile.writeLine("        materialInstanceId: " + FileInfo.baseName(tile.imageFileName));
                    saveSpriteMaterialInstance(
                        FileInfo.path(fileName),
                        FileInfo.baseName(tile.imageFileName),
                        FileInfo.fileName(tile.imageFileName));

                    minX = 0.0;
                    minY = 0.0;

                    maxX = 1.0;
                    maxY = 1.0;

                } else {
                    tileObjectFile.writeLine("        materialInstanceId: " + FileInfo.baseName(tileset.image));
                    let column = tile.id % columns;
                    let row = Math.trunc(tile.id / columns);

                    let x0 = column * (tileset.tileWidth + tileset.tileSpacing);
                    let y0 = row * (tileset.tileHeight + tileset.tileSpacing);

                    let x1 = x0 + tileset.tileWidth;
                    let y1 = y0 + tileset.tileHeight;

                    minX = 0.001 + x0 / tileset.imageWidth;
                    minY = 0.001 + y0 / tileset.imageHeight;

                    maxX = -0.001 + x1 / tileset.imageWidth;
                    maxY = -0.001 + y1 / tileset.imageHeight;
                }

                tileObjectFile.writeLine("        uv:");
                tileObjectFile.writeLine("          min:");
                tileObjectFile.writeLine("            x: " + minX);
                tileObjectFile.writeLine("            y: " + minY);
                tileObjectFile.writeLine("          max:");
                tileObjectFile.writeLine("            x: " + maxX);
                tileObjectFile.writeLine("            y: " + maxY);
                tileObjectFile.writeLine("        halfSize:");

                if (tileset.image === "") {
                    let scaleDividerPropertyValue = tileset.property("ScaleDivider");
                    let scaleDivider = scaleDividerPropertyValue === undefined ? 1.0 : scaleDividerPropertyValue;

                    tileObjectFile.writeLine("            x: " + 0.5 * tile.width / scaleDivider);
                    tileObjectFile.writeLine("            y: " + 0.5 * tile.height / scaleDivider);

                } else {
                    tileObjectFile.writeLine("            x: 0.5");
                    tileObjectFile.writeLine("            y: 0.5");
                }

                let maskPropertyValue = tile.property("Mask");
                let mask = maskPropertyValue === undefined ? 1 : maskPropertyValue;

                tileObjectFile.writeLine("        layer: " + 0)
                tileObjectFile.writeLine("        visibilityMask: " + mask)

                if (tile.objectGroup != null) {
                    tileObjectFile.writeLine("  - component:");
                    tileObjectFile.writeLine("      type: RigidBody2dComponent");
                    tileObjectFile.writeLine("      content:");
                    tileObjectFile.writeLine("        objectId: 0");
                    tileObjectFile.writeLine("        type: 0");

                    tile.objectGroup.objects.forEach(collisionShape => {
                        tileObjectFile.writeLine("  - component:");
                        tileObjectFile.writeLine("      type: CollisionShape2dComponent");
                        tileObjectFile.writeLine("      content:");
                        tileObjectFile.writeLine("        objectId: 0");
                        tileObjectFile.writeLine("        shapeId: " + collisionShape.id);
                        tileObjectFile.writeLine("        translation:");
                        tileObjectFile.writeLine("          x: " +
                            ((collisionShape.x + collisionShape.width / 2.0) / tile.width - 0.5));
                        tileObjectFile.writeLine("          y: " +
                            (0.5 - (collisionShape.y + collisionShape.height / 2.0) / tile.height));
                        tileObjectFile.writeLine("        rotation: " + collisionShape.rotation);
                        tileObjectFile.writeLine("        geometry:");

                        if (collisionShape.shape === MapObject.Rectangle) {
                            tileObjectFile.writeLine("          type: 0");
                            tileObjectFile.writeLine("          boxHalfExtents:");
                            tileObjectFile.writeLine("            x: " + (collisionShape.width / 2.0) / tile.width);
                            tileObjectFile.writeLine("            y: " + (collisionShape.height / 2.0) / tile.height);

                        } else if (collisionShape.shape === MapObject.Ellipse) {
                            if (collisionShape.width !== collisionShape.height) {
                                tiled.alert("Only circles are supported! Tile \"" + tile.className +
                                    "\" has ellipse.", "Export error");
                            } else {
                                tileObjectFile.writeLine("          type: 1");
                                tileObjectFile.writeLine("          circleRadius: " + collisionShape.width / 2.0);
                            }

                        } else {
                            tiled.alert("Only rectangle and ellipse collision shapes are supported! Tile \"" +
                                tile.className + "\" has other.", "Export error");
                        }

                        let materialPropertyValue = collisionShape.property("Material");
                        let material = materialPropertyValue === undefined ? "Default" : materialPropertyValue;
                        tileObjectFile.writeLine("        materialId: " + material);

                        let triggerPropertyValue = collisionShape.property("Trigger");

                        let trigger = triggerPropertyValue === undefined ? 0 : (triggerPropertyValue === true ? 1 : 0);
                        tileObjectFile.writeLine("        trigger: " + trigger);

                        let collisionGroupPropertyValue = collisionShape.property("CollisionGroup");
                        let collisionGroup = collisionGroupPropertyValue === undefined ?
                            0 : collisionGroupPropertyValue;
                        tileObjectFile.writeLine("        collisionGroup: " + collisionGroup);
                    });
                }

                tileObjectFile.commit();
            });
        }
    });

tiled.registerMapFormat(
    "EmergenceAssemblyMap",
    {
        name: "Emergence Assembly Library",
        extension: "stub",

        write: (tilemap, fileName) => {
            if (!fileName.endsWith("/EmergenceResourcesRoot.stub")) {
                tiled.alert("Incorrect save name! EmergenceResourcesRoot.stub file expected!", "Export failed")
                tiled.log(fileName)
                return;
            }

            let mapName = tilemap.className;
            let libraryDirectory = FileInfo.path(fileName) + "/Levels/" + mapName + "/";
            File.makePath(libraryDirectory);
            let folderDependenciesFile = new TextFile(
                libraryDirectory + "ObjectFolderDependencies.yaml", TextFile.WriteOnly);

            folderDependenciesFile.writeLine("list:")
            tilemap.usedTilesets().forEach(tileset => {
                folderDependenciesFile.writeLine("  - relativePath: \"../../Objects/" + tileset.name + "\"")
            });

            let customDependencyIndex = 0;
            let resolvedProperties = tilemap.resolvedProperties();

            while (resolvedProperties["CustomDependency" + customDependencyIndex] != null) {
                folderDependenciesFile.writeLine("  - relativePath: \"" +
                    resolvedProperties["CustomDependency" + customDependencyIndex] + "\"");
                ++customDependencyIndex;
            }

            folderDependenciesFile.commit();

            let levelObjectFile = new TextFile(
                libraryDirectory + mapName + ".object.yaml", TextFile.WriteOnly);
            levelObjectFile.writeLine("parent: \"\"");
            levelObjectFile.writeLine("changelist:");

            let layers = extractLayers(tilemap);
            let layerIndex = 0;
            let objectId = 1;

            layers.forEach(layer => {
                if (layer.isImageLayer) {
                    tiled.alert("Image layers are not supported yet!");

                } else if (layer.isObjectLayer) {
                    layer.objects.forEach(object => {
                        let locationX = (object.width * 0.5 + object.x + layer.offset.x) / tilemap.tileWidth;
                        let locationY = (object.height * 0.5 - object.y - layer.offset.y) / tilemap.tileHeight;
                        let className = null;

                        if (object.tile != null && object.tile.className != null && object.tile.className !== "") {
                            className = object.tile.className;
                        } else if (object.className != null && object.className !== "") {
                            className = object.className;
                        }

                        if (className != null && className !== "") {
                            saveMapObject(
                                levelObjectFile,
                                objectId,
                                className,
                                locationX,
                                locationY,
                                object.rotation,
                                layerIndex);
                            ++objectId;
                        }
                    });

                } else if (layer.isTileLayer) {
                    for (let x = -layer.width; x < layer.width; ++x) {
                        for (let y = -layer.height; y < layer.height; ++y) {
                            let tile = layer.tileAt(x, y);
                            if (tile != null && tile.className != null && tile.className !== "") {
                                let locationX = x + 0.5 + layer.offset.x / tilemap.tileWidth;
                                let locationY = -y - 0.5 - layer.offset.y / tilemap.tileHeight;

                                saveMapObject(
                                    levelObjectFile,
                                    objectId,
                                    tile.className,
                                    locationX,
                                    locationY,
                                    0.0,
                                    layerIndex);
                                ++objectId;
                            }
                        }
                    }
                }

                ++layerIndex;
            });

            levelObjectFile.commit();
        }
    }
);
