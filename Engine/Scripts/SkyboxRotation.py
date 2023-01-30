import engine

class SkyboxRotation(engine.ObjectScript):
    def Start(self):
        self.rotationSpeed = 0.01
        self.rotationAxes  = engine.Vector3(0, 1, 0)

    def Update(self):
        self.transform.Rotate(self.rotationAxes * self.rotationSpeed * self.time.DeltaTime())
