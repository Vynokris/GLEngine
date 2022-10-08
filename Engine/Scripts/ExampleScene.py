import engine

class ExampleScene:
    def __init__(self, app : engine.App):
        self.app = app

    def Load(self):
        app = self.app

        # ----- Create the camera ----- #

        camera = app.cameraManager.Create(engine.CameraParams(app.GetWindowWidth(), app.GetWindowHeight(), 0.1, 1000., 80.), True)
        app.cameraManager.sceneCamera = camera

        # ----- Setup scene lights ----- #

        light0 = app.lightManager.CreateDirLight()
        light1 = app.lightManager.CreateDirLight()
        light2 = app.lightManager.CreatePointLight()
        light3 = app.lightManager.CreateSpotLight()
        light4 = app.lightManager.CreatePointLight()
        light5 = app.lightManager.CreatePointLight()

        light0.SetParams(engine.RGB(0, 0, 0), engine.RGB(0.7, 0.7, 0.7), engine.RGB(0, 0, 0), engine.Vector3(-0.5, -1, 0.5))
        light1.SetParams(engine.RGB(0, 0, 0), engine.RGB(0.7, 0.7, 0.7), engine.RGB(0, 0, 0), engine.Vector3(0.5, 1, -0.5))
        light2.SetParams(engine.RGB(0, 0, 0), engine.RGB(1, 1, 1),       engine.RGB(1, 1, 1), 1, 0.025, 0.01, engine.Vector3(0, 1, -1))
        light3.SetParams(engine.RGB(0, 0, 0), engine.RGB(1, 1, 1),       engine.RGB(1, 1, 1), engine.Vector3(0, 6.8, 0), engine.Vector3(0, -1, 0), engine.PI/8, engine.PI/12)
        light4.SetParams(engine.RGB(0, 0, 0), engine.RGB(1, 0, 0),       engine.RGB(1, 0, 0), 1, 0.025, 0.05, engine.Vector3(-4.5, 9.5, 15.5))
        light5.SetParams(engine.RGB(0, 0, 0), engine.RGB(0.5, 0, 0),     engine.RGB(0.5, 0, 0), 1, 0.025, 0.03, engine.Vector3(-15, 14, 13))

        
        # ----- Setup scene graph ----- #

        # Camera and lights.
        sceneCamera = app.sceneGraph.AddCamera    ("Camera",           camera)
        dirLight    = app.sceneGraph.AddDirLight  ("DirectionalLight", light0)
        dirLight2   = app.sceneGraph.AddDirLight  ("DirectionalLight", light1)
        pointLight  = app.sceneGraph.AddPointLight("PointLight",       light2)
        spotLight   = app.sceneGraph.AddSpotLight ("SpotLight",        light3)

        # Models.
        player      = app.sceneGraph.AddModel("Player",       app.resourceManager.GetMesh("mesh_Rico"))
        gunLeft     = app.sceneGraph.AddModel("Gun",          app.resourceManager.GetMesh("mesh_Gun") , player)
        gunRight    = app.sceneGraph.AddModel("Gun",          app.resourceManager.GetMesh("mesh_Gun") , player)
        stadium     = app.sceneGraph.AddModel("Stadium",      app.resourceManager.GetMesh("stadium"))
        moon        = app.sceneGraph.AddModel("Moon",         app.resourceManager.GetMesh("mesh_moon"))
        headcrab    = app.sceneGraph.AddModel("Headcrab",     app.resourceManager.GetMesh("mesh_headcrab"))
        crowbar     = app.sceneGraph.AddModel("Crowbar",      app.resourceManager.GetMesh("mesh_crowbar"), headcrab)
        doomSlayer  = app.sceneGraph.AddModel("Doom Slayer",  app.resourceManager.GetMesh("mesh_doommarine"))
        bfg         = app.sceneGraph.AddModel("BFG",          app.resourceManager.GetMesh("mesh_bfg"), doomSlayer)
        masterChief = app.sceneGraph.AddModel("Master Chief", app.resourceManager.GetMesh("mesh_MasterChief"))
        energySword = app.sceneGraph.AddModel("Energy Sword", app.resourceManager.GetMesh("energySword"), masterChief)
        itemBox     = app.sceneGraph.AddModel("Item Box",     app.resourceManager.GetMesh("mesh_box"))
        banana      = app.sceneGraph.AddModel("Banana",       app.resourceManager.GetMesh("mesh_banana"), itemBox)
        palutena    = app.sceneGraph.AddModel("Palutena",     app.resourceManager.GetMesh("mesh_palutena"))
        staff       = app.sceneGraph.AddModel("Staff",        app.resourceManager.GetMesh("mesh_palutenaStaff"), palutena)
        alduin      = app.sceneGraph.AddModel("Alduin",       app.resourceManager.GetMesh("Alduin"))
        pointLight2 = app.sceneGraph.AddPointLight("PointLight", light4, alduin)
        pointLight3 = app.sceneGraph.AddPointLight("PointLight", light5, alduin)
        killZone    = app.sceneGraph.AddEmpty     ("KillZone")

        # Primitives.
        primCube0 = app.sceneGraph.AddPrimitive("Cube0",  engine.Primitive(engine.PrimitiveTypes.Cube))
        primCube1 = app.sceneGraph.AddPrimitive("Cube2",  engine.Primitive(engine.PrimitiveTypes.Cube)) 
        primCube2 = app.sceneGraph.AddPrimitive("Cube3",  engine.Primitive(engine.PrimitiveTypes.Cube))
        primCube3 = app.sceneGraph.AddPrimitive("Cube4",  engine.Primitive(engine.PrimitiveTypes.Cube))
        primCube4 = app.sceneGraph.AddPrimitive("Cube5",  engine.Primitive(engine.PrimitiveTypes.Cube))
        primCube5 = app.sceneGraph.AddPrimitive("Cube6",  engine.Primitive(engine.PrimitiveTypes.Cube))
        primCube6 = app.sceneGraph.AddPrimitive("Cube7",  engine.Primitive(engine.PrimitiveTypes.Cube))
        primCube7 = app.sceneGraph.AddPrimitive("Cube8",  engine.Primitive(engine.PrimitiveTypes.Cube))
        primCube8 = app.sceneGraph.AddPrimitive("Cube9",  engine.Primitive(engine.PrimitiveTypes.Cube))
        primCube9 = app.sceneGraph.AddPrimitive("Cube10", engine.Primitive(engine.PrimitiveTypes.Cube))
                                            
        # Primitive textures.
        primCube0.primitive.SetMaterial(app.resourceManager.GetMaterial("SteveMaterial"))
        primCube1.primitive.SetMaterial(app.resourceManager.GetMaterial("SteveMaterial"))
        primCube2.primitive.SetMaterial(app.resourceManager.GetMaterial("SteveMaterial"))
        primCube3.primitive.SetMaterial(app.resourceManager.GetMaterial("SteveMaterial"))
        primCube4.primitive.SetMaterial(app.resourceManager.GetMaterial("SteveMaterial"))
        primCube5.primitive.SetMaterial(app.resourceManager.GetMaterial("SteveMaterial"))
        primCube6.primitive.SetMaterial(app.resourceManager.GetMaterial("SteveMaterial"))
        primCube7.primitive.SetMaterial(app.resourceManager.GetMaterial("SteveMaterial"))
        primCube8.primitive.SetMaterial(app.resourceManager.GetMaterial("SteveMaterial"))
        primCube9.primitive.SetMaterial(app.resourceManager.GetMaterial("SteveMaterial"))

        # Colliders.
        cubeCollider0    = app.sceneGraph.AddCollider(primCube0, engine.PrimitiveTypes.Cube)
        cubeCollider1    = app.sceneGraph.AddCollider(primCube1, engine.PrimitiveTypes.Cube)
        cubeCollider2    = app.sceneGraph.AddCollider(primCube2, engine.PrimitiveTypes.Cube)
        cubeCollider3    = app.sceneGraph.AddCollider(primCube3, engine.PrimitiveTypes.Cube)
        cubeCollider4    = app.sceneGraph.AddCollider(primCube4, engine.PrimitiveTypes.Cube)
        cubeCollider5    = app.sceneGraph.AddCollider(primCube5, engine.PrimitiveTypes.Cube)
        cubeCollider6    = app.sceneGraph.AddCollider(primCube6, engine.PrimitiveTypes.Cube)
        cubeCollider7    = app.sceneGraph.AddCollider(primCube7, engine.PrimitiveTypes.Cube)
        cubeCollider8    = app.sceneGraph.AddCollider(primCube8, engine.PrimitiveTypes.Cube)
        cubeCollider9    = app.sceneGraph.AddCollider(primCube9, engine.PrimitiveTypes.Cube)
        playerCollider   = app.sceneGraph.AddCollider(player,    engine.PrimitiveTypes.Capsule , engine.Vector3(0, 0.25, 0), engine.Vector3(0, 0, 0), engine.Vector3(0.25, 0.25, 0.25))
        bananaCollider   = app.sceneGraph.AddCollider(banana,    engine.PrimitiveTypes.Cube)
        killZoneCollider = app.sceneGraph.AddCollider(killZone,  engine.PrimitiveTypes.Cube, engine.Vector3(0, 0, 0) , engine.Vector3(0, 0, 0), engine.Vector3(200, 1, 200))

        # Rigidbodies.
        player  .AddRigidbody()
        killZone.AddRigidbody(True)

        # Transform scene objects.
        player     .transform.SetScale(engine.Vector3(3, 3, 3))
        player     .transform.Move    (engine.Vector3(-2, 0, 0))
        player     .transform.SetRotation(engine.Vector3(0, engine.PI / 2 , 0))
        gunLeft    .transform.SetPosition(engine.Vector3(0.3 , 0.38 , 0))
        gunLeft    .transform.SetScale(engine.Vector3(0.05, 0.05, 0.05))
        gunRight   .transform.SetPosition(engine.Vector3(-0.3 , 0.38 , 0))
        gunRight   .transform.SetScale(engine.Vector3(0.05, 0.05, 0.05))
        camera     .transform.Move    (engine.Vector3(0, 1, -8))
        camera     .transform.Rotate  (engine.Vector3(engine.degToRad(15), 0, 0))
        primCube0  .transform.Move    (engine.Vector3(19.7, 2,     0.5 ))
        primCube0  .transform.SetScale(engine.Vector3(4.5,  1,     12.1))
        primCube1  .transform.Move    (engine.Vector3(26.4, 3.8,  0    ))
        primCube1  .transform.SetScale(engine.Vector3(3.6,  0.25, 4.9 ))
        primCube2  .transform.Move    (engine.Vector3(0,    -1,     0    ))
        primCube2  .transform.SetScale(engine.Vector3(23.5, 1,     7.15))
        primCube3  .transform.Move    (engine.Vector3(14.6, 0,     0.9 ))
        primCube3  .transform.SetScale(engine.Vector3(1.85, 1,     3.7 ))
        primCube4  .transform.Move    (engine.Vector3(29.9, 4.9,  11.3))
        primCube4  .transform.SetScale(engine.Vector3(3.6,  0.25, 4.9 ))
        primCube5  .transform.Move    (engine.Vector3(33.9, 2.9,  3.7 ))
        primCube5  .transform.SetScale(engine.Vector3(3.6,  0.25, 4.9 ))
        primCube6  .transform.Move    (engine.Vector3(23.8, 6.6,  14.6))
        primCube6  .transform.SetScale(engine.Vector3(3.6,  0.25, 4.9 ))
        primCube7  .transform.Move    (engine.Vector3(17.1, 7,     24.2))
        primCube7  .transform.SetScale(engine.Vector3(3.6,  0.25, 4.9 ))
        primCube8  .transform.Move    (engine.Vector3(17.2, 7.1,  33   ))
        primCube8  .transform.SetScale(engine.Vector3(6.6,  1.35, 5.65))
        primCube9  .transform.Move    (engine.Vector3(17.95, 3.975, 1))
        primCube9  .transform.SetScale(engine.Vector3(1, 2.95, 1))
        killZone   .transform.Move    (engine.Vector3(0, -3.9, 0))
        stadium    .transform.Move    (engine.Vector3(0, -3, 0))
        stadium    .transform.SetScale(engine.Vector3(4, 4, 4))
        moon       .transform.Move    (engine.Vector3(0, 40, 40))
        moon       .transform.Rotate  (engine.Vector3(0, engine.PI/2, -6*engine.PI/9))
        moon       .transform.SetScale(engine.Vector3(0.01, 0.01, 0.01))
        headcrab   .transform.Move    (engine.Vector3(-4.5, 1.6, 5))
        headcrab   .transform.Rotate  (engine.Vector3(0, engine.PI/2, 0))
        headcrab   .transform.SetScale(engine.Vector3(3, 3, 3))
        crowbar    .transform.Move    (engine.Vector3(0, 0.75, 0))
        crowbar    .transform.Rotate  (engine.Vector3(-engine.PI/3, engine.PI, -engine.PI/2))
        crowbar    .transform.SetScale(engine.Vector3(0.03, 0.03, 0.03))
        itemBox    .transform.Move    (engine.Vector3(-4.5, 0, 5))
        itemBox    .transform.SetScale(engine.Vector3(0.7, 0.7, 0.7))
        banana     .transform.Rotate  (engine.Vector3(engine.PI/2, -engine.PI/1.2, 0))
        banana     .transform.SetScale(engine.Vector3(0.02, 0.02, 0.02))
        doomSlayer .transform.Move    (engine.Vector3(-1.5, 0, 5))
        doomSlayer .transform.SetScale(engine.Vector3(0.02, 0.02, 0.02))
        bfg        .transform.Move    (engine.Vector3(0, 250, 0))
        bfg        .transform.Rotate  (engine.Vector3(0, engine.PI/2, 0))
        masterChief.transform.Move    (engine.Vector3(1.5, 0, 5))
        masterChief.transform.SetScale(engine.Vector3(0.35, 0.35, 0.35))
        energySword.transform.Move    (engine.Vector3(0, 13.85, 0))
        energySword.transform.Rotate  (engine.Vector3(engine.PI/2, engine.PI/2, 0))
        energySword.transform.SetScale(engine.Vector3(0.17, 0.17, 0.17))
        palutena   .transform.Move    (engine.Vector3(4.5, 0, 5))
        palutena   .transform.SetScale(engine.Vector3(9, 9, 9))
        staff      .transform.Move    (engine.Vector3(0, 0.54, 0))
        staff      .transform.Rotate  (engine.Vector3(0, 0, engine.PI/2))
        alduin     .transform.Move    (engine.Vector3(-20, 1.5, 24))
        alduin     .transform.Rotate  (engine.Vector3(0, engine.PI/6, -engine.PI/18))
        alduin     .transform.SetScale(engine.Vector3(0.05, 0.05, 0.05))

        # Update bounding boxes.
        cubeCollider0 .boundingSphere.Update(cubeCollider0,  primCube0)
        cubeCollider1 .boundingSphere.Update(cubeCollider1,  player   )
        cubeCollider2 .boundingSphere.Update(cubeCollider2,  primCube2)
        cubeCollider3 .boundingSphere.Update(cubeCollider3,  primCube3)
        cubeCollider4 .boundingSphere.Update(cubeCollider4,  primCube4)
        cubeCollider5 .boundingSphere.Update(cubeCollider5,  primCube5)
        cubeCollider6 .boundingSphere.Update(cubeCollider6,  primCube6)
        cubeCollider7 .boundingSphere.Update(cubeCollider7,  primCube7)
        cubeCollider8 .boundingSphere.Update(cubeCollider8,  primCube8)
        cubeCollider9 .boundingSphere.Update(cubeCollider9,  primCube9)
        bananaCollider.boundingSphere.Update(bananaCollider, banana)

        # Scripts.
        player     .AddPyScript("Scripts/Player.py",       app)
        sceneCamera.AddPyScript("Scripts/CameraScript.py", app)
        killZone   .AddPyScript("Scripts/KillZone.py",     app)