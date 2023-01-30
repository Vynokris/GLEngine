import engine

class RotateObject(engine.ObjectScript):
    def Start(self):
        self.rotationSpeed = 0.1

    def Update(self):
        self.transform.Rotate(-self.transform.Up() * self.rotationSpeed * self.time.DeltaTime())
