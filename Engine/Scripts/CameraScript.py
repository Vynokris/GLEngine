import engine

class CameraScript(engine.ObjectScript):
    def LateStart(self):
        self.speed      = 0.2
        self.lookAtDist = 5
        self.player     = self.app.sceneGraph.Find("Player")

    def Update(self):
        if self.player is not None:
            self.FollowPlayer()

    def FollowPlayer(self):
        playerPos = self.player.transform.GetPosition()
        camRot    = self.transform.GetRotation()

        if self.inputs.mouseRightClick:
            mouseDelta = self.inputs.mouseDelta

            # Rotate the camera with mouse delta.
            self.transform.Rotate(engine.Vector3(mouseDelta.y, -mouseDelta.x, 0) * self.speed * self.time.DeltaTime())
            camRot = self.transform.GetRotation()

            # Make it impossible for the camera to go upside down.
            if (camRot.x >  engine.PI/2):
                self.transform.SetRotation(engine.Vector3( engine.PI/2, camRot.y, camRot.z))
            if (camRot.x < -engine.PI/2):
                self.transform.SetRotation(engine.Vector3(-engine.PI/2, camRot.y, camRot.z))
            camRot = self.transform.GetRotation()

            # Change the camera distance according to the scroll wheel.
            self.lookAtDist -= self.inputs.mouseScroll / 5

            # Hide cursor and disable its movement.
            # engine.HideMouseCursor()
            # self.inputs.mousePos = self.inputs.mousePos - self.inputs.mouseDelta
            # self.inputs.SetMousePos(self.app, self.inputs.mousePos - self.inputs.mouseDelta)

        # else:
            # engine.ShowMouseCursor()

        # Move around the player according to the camera's rotation.
        camPos = playerPos - engine.Vector3FromAngles(engine.Vector3(-camRot.x, -camRot.y, camRot.z), self.lookAtDist)
        self.transform.SetPosition(camPos)
    