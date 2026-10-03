---
sidebar_position: 3
title: "Unreal Engine & Unity"
description: "Integrating PWSV with Unreal Engine 5 and Unity 3D for interactive audio-driven virtual worlds."
---

# Unreal Engine & Unity Integration

Game developers and virtual production artists can drive shaders, Niagara particles, and lighting systems in **Unreal Engine 5** and **Unity** directly from a DAW live session.

---

## 1. Unreal Engine 5 (UE5)

### Using the WebSocket Plugin
1. In UE5, enable the built-in **WebSockets** engine plugin (`Edit -> Plugins -> WebSockets`).
2. In C++ or Blueprints (via WebSocket wrapper nodes), connect to `ws://127.0.0.1:8080`.

### C++ Code Snippet
```cpp
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "Json.h"

void AMyAudioVisualizer::BeginPlay() {
    Super::BeginPlay();

    const FString ServerURL = TEXT("ws://127.0.0.1:8080");
    WebSocket = FWebSocketsModule::Get().CreateWebSocket(ServerURL);

    WebSocket->OnMessage().AddLambda([this](const FString& MessageString) {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(MessageString);

        if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
            double BPM = JsonObject->GetNumberField(TEXT("bpm"));
            bool bPlaying = JsonObject->GetBoolField(TEXT("playing"));
            double Beat = JsonObject->GetNumberField(TEXT("beat"));

            // Dispatch to Niagara Particle Parameters or Dynamic Material Instances
            OnBeatTick(Beat, BPM, bPlaying);
        }
    });

    WebSocket->Connect();
}
```

---

## 2. Unity 3D

In Unity, use `NativeWebSocket` or `System.Net.WebSockets`:

```csharp
using UnityEngine;
using NativeWebSocket;
using System;

[System.Serializable]
public class PWSVData {
    public int protocol;
    public double bpm;
    public int bar;
    public double beat;
    public bool playing;
}

public class PWSVReceiver : MonoBehaviour {
    private WebSocket websocket;

    async void Start() {
        websocket = new WebSocket("ws://127.0.0.1:8080");
        websocket.OnMessage += (bytes) => {
            string message = System.Text.Encoding.UTF8.GetString(bytes);
            PWSVData data = JsonUtility.FromJson<PWSVData>(message);
            // Drive Light component intensity, camera shake, or post-processing
        };
        await websocket.Connect();
    }

    void Update() {
        #if !UNITY_WEBGL || UNITY_EDITOR
        websocket.DispatchMessageQueue();
        #endif
    }

    private async void OnApplicationQuit() {
        await websocket.Close();
    }
}
```
