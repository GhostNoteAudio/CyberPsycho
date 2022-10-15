// #include "audio_yield.h"
//#include "audio_io.h"

// extern AudioIo audio;

// std::function<void(DataBuffer* data)> HandleAudio = 0;

// void yieldAudio()
// {
//     return;
//     if (audio.Available())
//     {
//         auto buf = audio.BeginAudioProcessing();
//         HandleAudio(buf);
//         audio.EndAudioProcessing();
//     }
// }




// this is new loop state

// void loop()
// {
//     pt.Start();
//     if (execPrint.Go())
//     {
//         if (audio.BufferUnderrun)
//         {
//             LogInfo("Buffer Underrun!!");
//             audio.BufferUnderrun = false;
//         }

//         LogInfof("CPU load: %.2f%%", Timers::GetCpuLoad()*100);

//         LogInfof("time : %f", pt.Period());
//         LogInfof("time avg: %f", pt.PeriodAvg());
//         LogInfof("time max: %f", pt.PeriodMax());
//     }

//     if (updateState.Go())
//     {
//         controls.UpdatePotState(0);
//         controls.UpdatePotState(1);
//     }

//     yieldAudio();

//     if (updateMenu.Go())
//     {
//         auto p0 = controls.GetPot(0);
//         auto p1 = controls.GetPot(1);
//         menuManager.HandlePotUpdate(0, p0.Value);
//         if (p1.IsNew)
//             menuManager.HandlePotUpdate(1, p1.Value);

//         menuManager.Render();
//     }

//     if (master.finished())
//     {
//         menuManager.Transfer();
//     }
    
//     pt.Stop();
// }
