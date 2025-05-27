package entity;

import cgr.Log;

class Matter {
    
    public function onBegin():Void {
        Log.info("Matter started");
        
    }
    
    public function onUpdate():Void {
        Log.trace("Updating Matter");
    }
}