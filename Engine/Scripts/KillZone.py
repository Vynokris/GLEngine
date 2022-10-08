import engine

class KillZone(engine.ObjectScript):
    def LateStart(self):
        self.player = self.app.sceneGraph.Find("Player")

    def OnCollisionEnter(self):
        if self.player.rigidbody is not None:
            engine.DebugLog("The player entered the killzone.", __file__)
            self.player.transform.SetPosition(self.player.GetPyScript(0).initialPos)
            self.player.transform.SetRotation(engine.Vector3(0, 0, 0))
            self.player.rigidbody.SetVelocity(engine.Vector3(0, 0, 0))
            self.player.rigidbody.SetAcceleration(engine.Vector3(0, 0, 0))