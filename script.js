function detectPhaseShift(sig, res, sigMax, resMax) {
    print("Given sig: " + sig + " Given res: " + res + " Given sigMax: " + sigMax + " Given resMax: " + resMax);
    if(sigMax - sig < sigMax && resMax - res > resMax) {
        return true;
    }
    if(sigMax - sig > sigMax && resMax - res < resMax) {
        return true;
     }
    print("nope")
    return false;
}

function doScopeWavegen(){
    print("Running Scope and Wavegen script");
    Wavegen1.Channel1.Mode.text = "Simple";
    Wavegen1.Channel1.Simple.Offset.value = 0.5;
    Scope1.Trigger.Trigger.text = "Repeated";
    Wavegen1.run();
    Scope1.run();
    StaticIO.Channel0.Mode.text = "I/Os";
    StaticIO.Channel0.DIO0.Mode.text = "LED";
    StaticIO.Channel0.DIO0.Mode.text = "LED";
    StaticIO.Channel0.DIO0.Mode.text = "LED";
    StaticIO.Channel0.DIO0.Mode.text = "LED";
    StaticIO.Channel0.DIO0.text = "0";
    StaticIO.Channel0.DIO1.text = "0";
    StaticIO.Channel0.DIO2.text = "0";
    StaticIO.Channel0.DIO3.text = "0";
    for(var idx = 0; wait(1) && idx < 10000000; idx++){
        if(!Scope1.wait()){
            return;
        }
        var signalMax = Scope1.Channel1.measure("Maximum");
        var signalMin = Scope1.Channel1.measure("Minimum");
        var responseMax = Scope1.Channel2.measure("Maximum");
        var responseMin = Scope1.Channel2.measure("Minimum");
        print("Signal Max: " + signalMax + "  Response Max: " + responseMax + " Diff : " + (signalMax - responseMax));
        print("Response Min: " + responseMin);

        // Detect Inductor
        if(signalMax - responseMax < 0.1 && signalMax - responseMax > -0.1 && responseMin < -0.1) {
            StaticIO.Channel0.DIO3.text = "1";
        } else {
            StaticIO.Channel0.DIO3.text = "0";
        }

        // Detect Diode
        if(responseMax > 0.01 && responseMin < 0.05 && responseMin > -0.05) {
            StaticIO.Channel0.DIO1.text = "1";
        } else {
            StaticIO.Channel0.DIO1.text = "0";
        }

        // Detect Resistor
        if(signalMax - responseMax < signalMax && signalMin - responseMin > signalMin
            && responseMax > 0.01 && responseMin < -0.05 && signalMax - responseMax > 0.01) {
            StaticIO.Channel0.DIO0.text = "1";
        } else {
            StaticIO.Channel0.DIO0.text = "0";
        }

        var numTruth = 0;
        // Detect Capacitor

        var sigData = Scope1.Channel1.data;
        var resData = Scope1.Channel2.data;
        //sigData.forEach(function(sample){print("Signal Sample: " + sample);})
        //resData.forEach(function(sample){print("Result Sample: " + sample);})


        for(var i = 0;  i < 100; i = i + 5) {
            if(detectPhaseShift(Scope1.Channel1.Sample(i), Scope1.Channel2.Sample(i), signalMax, responseMax)) {
                numTruth ++;
                print("FOUND A THING FOUND A THING!!!!!");
            }
        }
        if(numTruth > 0 && responseMin < -0.1) {
            StaticIO.Channel0.DIO2.text = "1";
        } else {
            StaticIO.Channel0.DIO2.text = "0";
        }

        // var average = Scope1.Channel1.measure("Average");
        var middle = Scope1.Channel1.measure("Middle");
        // adjust Wavegen offset
        Wavegen1.Channel1.Simple.Offset.value -= middle/2;
}
}
if(!('Wavegen1' in this) || !('Scope1' in this)) {
    throw("Please open a Scope and a Wavegen instrument");
}
doScopeWavegen();
