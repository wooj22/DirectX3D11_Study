# 🎨 DirectX 11 3D Rendering Engine

**DirectX 11 기반의 3D 자체 게임 엔진 렌더링 파이프라인 프로젝트입니다.**

3D 자체 엔진을 활용한 게임 개발 과정에서 **그래픽스 렌더링 시스템의 설계 및 구현을 담당**했습니다.

렌더러는 **PBR 기반 Hybrid Rendering (Deferred + Forward)** 구조로 설계하였으며,
Opaque / Transparent 객체의 특성에 따라 렌더링 경로를 분리하고 Multi-Pass Pipeline을 통해 Shadow, Lighting, Decal, Effect, HDR 및 Post Processing을 처리합니다.

---

## 📌 Renderer Overview

### Rendering Core

* DirectX 11
* PBR (Physically Based Rendering)
* Cook-Torrance BRDF
* Metallic-Roughness Workflow
* Hybrid Rendering

  * Deferred Rendering
  * Forward Rendering
* Multi-Pass Rendering Pipeline
* MRT 기반 G-Buffer

### Lighting & Shadow

* Directional Light
* Point Light
* Spot Light
* Image Based Lighting (IBL)
* Lighting Volume
* Stencil 기반 Lighting Volume 최적화
* Directional Shadow Mapping
* PCF (Percentage Closer Filtering)

### Material Texture

* Albedo
* Normal
* Roughness (Shininess)
* Metallic
* Emissive

### Surface Processing

* Screen Space Decal
* Stencil Masked Decal
* Geometry Based Outline Rendering

### Environment

* Skybox

### Effect

* Flipbook Effect
* Particle System

### HDR & Post Processing

* HDR Rendering
* Bloom

  * Prefilter
  * Downsample
  * Upsample
* Color Grading
* Film Grain
* Vignette
* Tone Mapping
* Gamma Correction

---

# 🏗 Renderer Architecture

본 렌더러는 **Hybrid Rendering (Deferred + Forward)** 구조를 기반으로 합니다.

Opaque 객체는 G-Buffer를 이용한 **Deferred Rendering**으로 처리하고,
Transparent 객체는 별도의 **Forward Rendering** 경로로 처리합니다.

```text
                    ┌──────────────────────┐
                    │      Render Scene    │
                    └──────────┬───────────┘
                               │
                  ┌────────────┴────────────┐
                  │                         │
                  ▼                         ▼
          Opaque Objects            Transparent Objects
                  │                         │
                  ▼                         │
        Deferred Rendering                  │
                  │                         │
          ┌───────┴───────┐                 │
          │               │                 │
          ▼               ▼                 │
       G-Buffer        Deferred             │
                       Lighting             │
          │               │                 │
          └───────┬───────┘                 │
                  │                         │
                  ▼                         ▼
             Scene Color  ◀──────  Forward Rendering
                  │
                  ▼
          Effect / Particle
                  │
                  ▼
                Bloom
                  │
                  ▼
           Post Processing
                  │
                  ▼
              BackBuffer
```

Deferred Rendering은 다수의 광원을 처리할 때 Geometry와 Lighting 연산을 분리할 수 있다는 장점을 가지지만, Alpha Blending이 필요한 Transparent 객체를 처리하기 어렵습니다.

따라서 렌더링 경로를

```text
Opaque       → Deferred Rendering
Transparent  → Forward Rendering
```

으로 분리하여 각각의 특성에 맞게 처리하도록 설계했습니다.

---

# 🚀 Render Pipeline

전체 렌더링 파이프라인은 다음 순서로 진행됩니다.

```text
1. Shadow Depth Only Pass
           │
           ▼
2. Geometry Pass
   (Opaque → G-Buffer)
           │
           ▼
3. Decal Pass
           │
           ▼
4. Deferred Lighting Pass
   ├─ Lighting Volume Stencil Pass
   └─ PBR Lighting Pass
           │
           ▼
5. Skybox Pass
           │
           ▼
6. Forward Transparent Pass
           │
           ▼
7. Effect / Particle Pass
           │
           ▼
8. Bloom Pass
   ├─ Prefilter
   ├─ Downsample Blur
   └─ Upsample Combine
           │
           ▼
9. PostProcess / ToneMapping Pass
           │
           ▼
       BackBuffer
```

---

# 1. Shadow Depth Only Pass

Lighting 이전에 Directional Light를 기준으로 Scene의 Depth 정보를 렌더링합니다.

```text
Scene Geometry
      │
      ▼
Light View / Projection
      │
      ▼
Depth Only Rendering
      │
      ▼
Shadow Depth Texture
```

생성된 Shadow Depth Texture는 이후 PBR Lighting 단계에서 Shadow 판정에 사용합니다.

Shadow 경계의 Aliasing을 완화하기 위해 **PCF (Percentage Closer Filtering)** 를 적용합니다.

---

# 2. Geometry Pass

Opaque 객체의 Geometry 정보를 MRT를 통해 여러 개의 Render Target에 기록하여 **G-Buffer**를 구성합니다.

```text
              Opaque Geometry
                     │
                     ▼
                Geometry Pass
                     │
             ┌───────┼───────┐
             ▼       ▼       ▼
          GBuffer  GBuffer  GBuffer
             │       │       │
             └───────┼───────┘
                     │
                     ▼
             Deferred Lighting
```

이 단계에서는 최종 Lighting 결과를 계산하지 않고, 이후 Lighting에 필요한 Surface 정보를 Screen Space Buffer에 저장합니다.

이를 통해 Geometry Rendering과 Lighting 계산을 분리합니다.

---

# 3. Decal Pass

Scene의 표면 위에 추가적인 Texture 정보를 표현하기 위한 Decal Rendering을 수행합니다.

구현된 방식은 다음과 같습니다.

```text
Screen Space Decal
Stencil Masked Decal
```

Screen Space 정보를 이용하여 이미 렌더링된 Geometry 표면에 Decal을 적용하며, Stencil Mask를 활용하여 Decal 적용 영역을 제한할 수 있도록 구성했습니다.

---

# 4. Deferred Lighting Pass

Geometry Pass에서 생성한 G-Buffer를 기반으로 PBR Lighting을 계산합니다.

지원하는 Light Type은 다음과 같습니다.

```text
Directional Light
Point Light
Spot Light
```

Lighting Pass 내부에서는 크게 다음 과정이 수행됩니다.

```text
G-Buffer
   │
   ├─────────────┐
   │             │
   ▼             ▼
Material       Geometry
Information   Information
   │             │
   └──────┬──────┘
          │
          ▼
   PBR Lighting
          │
          ▼
     Scene Color
```

---

## 💡 PBR Lighting

Lighting Model은 **Cook-Torrance BRDF** 기반의 PBR을 사용합니다.

Material은 Metallic-Roughness Workflow를 기반으로 구성합니다.

```text
Material
├─ Albedo
├─ Normal
├─ Roughness
├─ Metallic
└─ Emissive
```

이를 통해 Material의 물리적 특성을 기반으로 Lighting을 계산하도록 구성했습니다.

---

## 💡 Lighting Volume Optimization

Point Light / Spot Light와 같은 Local Light는 화면 전체 Pixel에 Lighting 연산을 수행할 필요가 없습니다.

따라서 Light의 영향 범위를 나타내는 Geometry를 이용하는 **Lighting Volume** 방식을 사용합니다.

```text
Point Light
     │
     ▼
Sphere Lighting Volume
     │
     ▼
Stencil Pass
     │
     ▼
Affected Pixels
     │
     ▼
PBR Lighting
```

Stencil Buffer를 이용해 실제 Light의 영향을 받는 영역을 제한하고, 해당 Pixel에 대해서만 Lighting Shader가 실행되도록 구성했습니다.

---

# 5. Skybox Pass

Scene의 Environment를 표현하기 위해 Skybox를 렌더링합니다.

또한 PBR 환경광 표현을 위해 **Image Based Lighting (IBL)** 을 사용합니다.

```text
Environment
    │
    ├─ Skybox
    │
    └─ IBL
         │
         ▼
   PBR Environment Lighting
```

---

# 6. Forward Transparent Pass

Alpha Blending이 필요한 Transparent 객체는 Deferred Rendering에서 분리하여 **Forward Rendering**으로 처리합니다.

```text
Opaque
   │
   ▼
Deferred Rendering
   │
   ▼
Scene Color
   │
   ├──────────── Transparent
   │                 │
   │                 ▼
   │          Forward Rendering
   │                 │
   ◀─────────────────┘
```

이를 통해 Deferred Rendering의 구조적인 Alpha Blending 문제를 해결하도록 설계했습니다.

---

# 7. Effect / Particle Pass

게임에서 사용하는 Effect는 별도의 Rendering Pass로 처리합니다.

구현된 Effect 기능은 다음과 같습니다.

```text
Effect
├─ Flipbook Animation
└─ Particle System
```

Scene Geometry 및 Transparent Rendering 이후 Effect를 합성하도록 구성했습니다.

---

# 8. Bloom Pass

HDR Scene에서 일정 밝기 이상의 영역을 추출하여 Bloom 효과를 적용합니다.

Bloom은 다음 Multi-Pass 구조로 처리합니다.

```text
HDR Scene
    │
    ▼
Prefilter
    │
    ▼
Bright Area
    │
    ▼
Downsample
    │
    ▼
Downsample
    │
    ▼
   ...
    │
    ▼
Upsample
    │
    ▼
Upsample + Combine
    │
    ▼
Bloom Texture
    │
    ▼
HDR Scene + Bloom
```

Bloom Pipeline은 크게 다음 세 단계로 구성됩니다.

### Prefilter

Bloom을 적용할 밝은 영역을 추출합니다.

### Downsample Blur

Texture Resolution을 단계적으로 낮추며 Blur 효과를 생성합니다.

### Upsample Combine

낮은 Resolution부터 다시 Upsample하며 이전 단계의 결과를 합성합니다.

이를 통해 넓게 퍼지는 Bloom 효과를 생성합니다.

---

# 9. Post Processing

최종 HDR Scene에 Post Processing을 적용합니다.

구현된 Post Processing 효과는 다음과 같습니다.

```text
HDR Scene
    │
    ├─ Bloom
    ├─ Color Grading
    ├─ Film Grain
    └─ Vignette
    │
    ▼
Tone Mapping
    │
    ▼
Gamma Correction
    │
    ▼
BackBuffer
```

최종적으로 Tone Mapping을 통해 HDR 색상 범위를 출력 가능한 영역으로 변환하고, Gamma Correction을 거쳐 BackBuffer에 렌더링합니다.

---

# 🔧 Render Pass & Resource Binding

각 Render Pass는 자신에게 필요한 **Pipeline State를 사전에 구성**합니다.

```text
Render Pass
     │
     ├─ Shader
     ├─ Render Target
     ├─ Depth / Stencil State
     ├─ Rasterizer State
     ├─ Blend State
     └─ etc...
```

이후 각 Renderable Object는 실제 Draw Call 시점에 객체별로 필요한 GPU Resource만 업데이트합니다.

```text
Renderable Object
     │
     ├─ Vertex Buffer
     ├─ Index Buffer
     ├─ Constant Buffer
     └─ Shader Resource View
             │
             ▼
          Draw Call
```

즉,

```text
Pass
 └─ Rendering Environment / Pipeline State 관리

Object
 └─ Object별 GPU Resource Binding
```

으로 책임을 분리했습니다.

이를 통해 Pass 단위의 렌더링 환경 설정과 Object 단위의 Resource Binding을 명확하게 분리하고, 렌더링 파이프라인의 가독성과 유지보수성을 높이는 것을 목표로 설계했습니다.

---

# 📊 Pipeline Summary

```text
                         [ Scene ]
                            │
                            ▼
                  Shadow Depth Pass
                            │
                            ▼
                    Geometry Pass
                            │
                            ▼
                       G-Buffer
                            │
                            ▼
                       Decal Pass
                            │
                            ▼
                 Deferred Lighting
                   ┌────────┴────────┐
                   │                 │
             Stencil Volume      PBR Lighting
                   │                 │
                   └────────┬────────┘
                            │
                            ▼
                         Skybox
                            │
                            ▼
                  Forward Transparent
                            │
                            ▼
                   Effect / Particle
                            │
                            ▼
                         HDR Scene
                            │
                            ▼
                          Bloom
                  ┌─────────┼─────────┐
                  │         │         │
             Prefilter  Downsample Upsample
                  │         │         │
                  └─────────┼─────────┘
                            │
                            ▼
                    Post Processing
                            │
                            ▼
                       Tone Mapping
                            │
                            ▼
                    Gamma Correction
                            │
                            ▼
                       BackBuffer
```

---

# 🛠 Tech Stack

| Category        | Technology                                   |
| --------------- | -------------------------------------------- |
| Language        | C++                                          |
| Graphics API    | DirectX 11                                   |
| Rendering       | Deferred + Forward Hybrid Rendering          |
| Shading         | PBR / Cook-Torrance BRDF                     |
| Material        | Metallic-Roughness Workflow                  |
| Lighting        | Directional / Point / Spot / IBL             |
| Shadow          | Shadow Mapping / PCF                         |
| Optimization    | Lighting Volume / Stencil                    |
| Post Processing | HDR / Bloom / Tone Mapping / Color Grading   |
| Effect          | Particle / Flipbook                          |
| Surface         | Screen Space Decal / Stencil Decal / Outline |

---

# 🎯 Project Focus

이 프로젝트에서는 단순히 DirectX 11 API를 이용하여 Mesh를 출력하는 것을 넘어, **실제 게임 엔진에서 사용할 수 있는 렌더링 파이프라인을 직접 설계하고 구현하는 것**을 목표로 했습니다.

특히 다음 부분을 중점적으로 구현했습니다.

* Deferred / Forward 각각의 특성을 고려한 **Hybrid Rendering Pipeline 설계**
* MRT와 G-Buffer를 활용한 **Deferred Rendering**
* Cook-Torrance BRDF 기반 **PBR Lighting**
* Directional / Point / Spot **Multi-Light 처리**
* Stencil Buffer와 Lighting Volume을 이용한 **Local Light 최적화**
* Shadow Mapping 및 **PCF Shadow**
* Screen Space / Stencil 기반 **Decal Rendering**
* Transparent Object를 위한 **Forward Rendering Path**
* Particle / Flipbook 기반 **Effect Rendering**
* Multi-Pass Downsample / Upsample 기반 **Bloom**
* HDR → Tone Mapping → Gamma Correction으로 이어지는 **Post Processing Pipeline**
* Render Pass와 Renderable Object의 역할을 분리한 **Rendering Architecture**
