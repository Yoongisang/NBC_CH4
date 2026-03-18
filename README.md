# NBC_CH4 - 멀티플레이어 숫자 야구 게임

Unreal Engine 5 C++ 기반의 멀티플레이어 채팅 + 숫자 야구 게임 프로젝트입니다.
플레이어들이 채팅을 통해 소통하며, 서버가 생성한 3자리 비밀 번호를 맞추는 숫자 야구 게임을 진행합니다.

---

## 과제 소개

- **주제**: 멀티플레이어 환경에서 채팅 시스템과 숫자 야구 게임을 결합한 네트워크 게임
- **목표**: UE5의 네트워크 리플리케이션(RPC, Replicated Property)을 활용하여 실시간 멀티플레이어 게임 구현
- **핵심 학습 요소**:
  - Server/Client RPC (ServerRPC, ClientRPC, MulticastRPC)
  - Property Replication (`GetLifetimeReplicatedProps`)
  - GameMode / GameState / PlayerState / PlayerController 아키텍처
  - UMG 위젯을 활용한 채팅 UI 구현

---

## 주요 기능

| 기능 | 설명 |
|------|------|
| **멀티플레이어 채팅** | 접속한 모든 플레이어에게 실시간 메시지 브로드캐스트 |
| **숫자 야구 게임** | 서버가 생성한 3자리 비밀번호(1~9, 중복 없음)를 맞추는 게임 |
| **Strike / Ball 판정** | 숫자와 위치 모두 일치 → Strike, 숫자만 일치 → Ball |
| **승리 / 무승부 판정** | 3 Strike 시 승리, 모든 플레이어가 최대 시도 횟수 소진 시 무승부 |
| **자동 게임 리셋** | 승리 또는 무승부 후 새로운 비밀번호 생성 및 시도 횟수 초기화 |
| **접속 알림** | 플레이어 입장 시 전체 알림 표시 (5초 후 자동 소멸) |

---

## 프로젝트 구조

```
Source/NBC_CH4/
├── Game/
│   ├── NBCGameModeBase.h/cpp      # 게임 로직 (비밀번호 생성, 판정, 게임 흐름 관리)
│   └── NBCGameStateBase.h/cpp     # 게임 상태 (로그인 브로드캐스트 MulticastRPC)
├── Player/
│   ├── NBCPlayerController.h/cpp  # 플레이어 입력 처리, 채팅 UI 관리, RPC 통신
│   └── NBCPlayerState.h/cpp       # 플레이어 상태 (이름, 시도 횟수, 리플리케이션)
└── UI/
    └── NBCChatInput.h/cpp         # 채팅 입력 위젯 (UMG EditableTextBox 바인딩)
```

---

## 클래스 설계

### ANBCGameModeBase (서버 전용)
- 비밀번호 생성 (`GenerateSecretNumber`) : 1~9 중 중복 없는 3자리
- 입력 유효성 검증 (`IsGuessNumberString`) : 3자리, 1~9, 중복 없음 확인
- Strike/Ball 판정 (`JudgeResult`) : 위치+숫자 일치 → Strike, 숫자만 일치 → Ball
- 게임 승리/무승부 판정 (`JudgeGame`) : 3 Strike → 승리, 전원 소진 → 무승부
- 채팅/추측 메시지 분기 처리 (`PrintChatMessageString`) : 숫자 입력 시 판정, 일반 텍스트는 채팅

### ANBCGameStateBase
- `MulticastRPCBroadcastLoginMessage` : 플레이어 접속 시 전체 클라이언트에 알림 전송

### ANBCPlayerController
- 채팅 UI 위젯 생성 및 관리 (BeginPlay에서 로컬 플레이어만 생성)
- `ServerRPCPrintChatMessageString` : 클라이언트 → 서버 메시지 전송
- `ClientRPCPrintChatMessageString` : 서버 → 클라이언트 메시지 표시
- 알림 텍스트 리플리케이션 (`NotificationText`)

### ANBCPlayerState
- `PlayerNameString` / `CurrentGuessCount` / `MaxGuessCount` 리플리케이션
- `GetPlayerInfoString()` : "PlayerName(현재/최대)" 형식 문자열 반환

### UNBCChatInput (UUserWidget)
- `EditableTextBox` 바인딩 (BindWidget)
- Enter 키 입력 시 `PlayerController::SetChatMessageString` 호출 후 입력란 초기화

---

## 네트워크 통신 흐름

```
[Client] 채팅 입력 (UNBCChatInput)
    │
    ▼
[Client] ANBCPlayerController::SetChatMessageString()
    │
    ▼ ServerRPC
[Server] ANBCPlayerController::ServerRPCPrintChatMessageString()
    │
    ▼
[Server] ANBCGameModeBase::PrintChatMessageString()
    │
    ├─ 유효한 숫자 추측 → 판정 결과 브로드캐스트 → 승리/무승부 체크
    ├─ 잘못된 숫자 형식 → 오류 메시지 전송
    └─ 일반 채팅 → 전체 브로드캐스트
    │
    ▼ ClientRPC (각 플레이어에게)
[Client] ANBCPlayerController::ClientRPCPrintChatMessageString()
    │
    ▼
[Client] 화면에 메시지 출력
```

---

## 구현 과정

### Step 1. 프로젝트 초기 설정 (2026-03-17)
- UE5 프로젝트 생성 및 기본 모듈 구성
- GameModeBase, GameStateBase, PlayerController, PlayerState, ChatInput 클래스 뼈대 생성
- 멀티플레이어를 위한 리플리케이션 기본 설정 (`bReplicates = true`)
- Blueprint 클래스 (BP_GameModeBase, BP_PlayerController) 및 맵(Chatting) 생성

### Step 2. 채팅 입력 UI 구현 (2026-03-19)
- `UNBCChatInput` 위젯에 `EditableTextBox` BindWidget 연동
- `OnTextCommitted` 델리게이트 바인딩/언바인딩 (NativeConstruct/NativeDestruct)
- Enter 키 입력 시 PlayerController로 메시지 전달 후 입력란 클리어
- PlayerController의 `BeginPlay`에서 로컬 플레이어 전용 위젯 생성 및 뷰포트 추가
- UI 전용 입력 모드 설정

### Step 3. 숫자 야구 게임 로직 구현 (2026-03-19)
- `GenerateSecretNumber()` : 1~9 중 랜덤 3자리 비밀번호 생성 (중복 배제)
- `IsGuessNumberString()` : 입력값 유효성 검증 (3자리, 1~9, 중복 없음)
- `JudgeResult()` : Strike/Ball 판정 로직
- `PrintChatMessageString()` : 채팅과 숫자 추측 분기 처리
- PlayerState에 `CurrentGuessCount` / `MaxGuessCount` 추가 및 리플리케이션
- 승리(3 Strike) / 무승부(전원 소진) 판정 및 자동 게임 리셋

### Step 4. 멀티플레이어 핵심 기능 완성 (2026-03-19)
- `ServerRPC` / `ClientRPC` 기반 채팅 메시지 송수신 구현
- `MulticastRPC` 기반 플레이어 접속 알림 브로드캐스트
- 알림 텍스트 위젯 (`WBP_NotificationText`) 추가 및 5초 타이머 자동 소멸
- `NotificationText` 프로퍼티 리플리케이션
- 전체 플레이어 관리 (`AllPlayerControllers` 배열) 및 게임 판정 고도화

---

## 기술 스택

- **엔진**: Unreal Engine 5
- **언어**: C++ / Blueprint
- **네트워크**: UE5 내장 리플리케이션 시스템 (RPC, Replicated Properties)
- **UI**: UMG (Unreal Motion Graphics)

---

## 실행 방법

1. Unreal Engine 5에서 프로젝트 열기
2. `Chatting` 맵 로드
3. 멀티플레이어 테스트: **Play** → **Number of Players: 2 이상** → **Net Mode: Listen Server**
4. 채팅창에 텍스트 입력 후 Enter → 일반 채팅
5. 3자리 숫자(1~9, 중복 없음) 입력 후 Enter → 숫자 야구 추측
