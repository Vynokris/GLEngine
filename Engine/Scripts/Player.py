import engine
from enum import IntEnum

class PlayerStates(IntEnum):
    Grounded = 0
    InAir    = 1

class Player(engine.ObjectScript):
    def Start(self):
        self.groundSpeed = 500
        self.airSpeed    = 350
        self.jumpForce   = 25000
        self.state       = PlayerStates.Grounded
        self.initialPos  = self.transform.GetPosition()
        self.camera      = self.app.sceneGraph.Find("Camera")

    def Update(self):
        if (self.inputs.movement != engine.Vector3(0, 0, 0)):
            self.transform.SetRotation(engine.Vector3(0, -self.camera.transform.GetRotation().y, 0))

        if self.state == PlayerStates.Grounded:
            self.GroundMovement()
        elif self.state == PlayerStates.InAir:
            self.AerialMovement()

    def OnCollisionEnter(self):
        if self.CheckRigidbody() and self.rigidbody.IsGrounded():
            self.state = PlayerStates.Grounded

    def OnCollisionExit(self):
        if self.CheckRigidbody() and not self.rigidbody.IsGrounded():
            self.state = PlayerStates.InAir

    def GroundMovement(self):
        if self.CheckCamera() and self.CheckRigidbody():
            movement = engine.Vector3(self.inputs.movement.x * self.groundSpeed, self.rigidbody.GetVelocity().y, self.inputs.movement.z * self.groundSpeed) * self.time.DeltaTime()
            self.rigidbody.SetVelocity(((movement.toVector4() * engine.GetRotationMatrix(engine.Vector3(0, self.camera.transform.GetRotation().y, 0))).toVector3(True)))
            if self.inputs.movement.y > 0:
                self.Jump()

    def AerialMovement(self):
        if self.CheckCamera() and self.CheckRigidbody():
            movement = engine.Vector3(self.inputs.movement.x * self.airSpeed, self.rigidbody.GetVelocity().y, self.inputs.movement.z * self.airSpeed) * self.time.DeltaTime()
            self.rigidbody.SetVelocity(((movement.toVector4() * engine.GetRotationMatrix(engine.Vector3(0, self.camera.transform.GetRotation().y, 0))).toVector3(True)))

    def Jump(self):
        self.rigidbody.AddAcceleration(engine.Vector3(0, self.jumpForce, 0) * self.time.DeltaTime())

    def CheckRigidbody(self):
        if self.rigidbody is None:
            engine.DebugLogWarning("No rigidbody found in Player.")
            return False
        return True

    def CheckCamera(self):
        if self.camera is None:
            engine.DebugLogWarning("Player was unable to find its camera.")
            return False
        return True
        