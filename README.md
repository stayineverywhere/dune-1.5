# 컴퓨터공학과 윤주영, 20232532

# 이중버퍼 구현과정
- 인터넷의 이중버퍼는 배열을 이용하여 변경된 내용을 복사하지만, 문서를 찾아보고 인터넷을 검색한 결과
윈도우에서 제공하는 conio HANDLE을 이용하여 구현할 수 있었습니다.
- 화면에 해당하는 STD_OUTPUT_HANDLE과 동일한 구조를 갖는 hidden buffer를 생성한 후, 
  모든 출력은 hidden buffer에 작성합니다.
- 화면 출력이 필요한 경우, hidden buffer와 console의 내용을 비교하여 hidden buffer의 내용이 
  변경되면, 그때 console로 복사합니다. 화면의 일부분만을 복사할 수도 있지만 텍스트 화면이기때문에
  전체 화면을 복사합니다. 

# intro/outtro 화면 생성
- 인트로/아웃트로는 듄의 이미지를 인터넷에서 텍스트로 변경하는 사이트 도움을 받아, 변환된 텍스트를 
  출력하는 것으로 하였습니다.

# 유닛을 구분하기 위하여 모든 유닛은 캐릭터 레이블을 가지고 있으며, 기지와 하베스터의 경우 사용자와 AI를 구분하기 위하여
  소문자/대문자를 이용하였습니다.
# 게임 진행 시간을 맵위의 우측 상단에 00:00:00 형식으로 표시하였습니다.

* 유닛이 선택되지 않은 경우에 SPACE는 사막 정보를 표시합니다.
* ESC를 누르면 초기 정보 (캐릭터 정보)를 출력합니다.
* 데모를 위하여, 샌드웜, 사막독수리, 사막폭풍은 원래 이동시간의 1/10로 단축하여 이동합니다.
* 보병, 프레멘, 투사, 중전차를 샌드웜의 공격을 위하여 미리 2개씩 배치하였습니다.

* 샌드웜 이동 과정
- 샌드웜을 유닛을 발견하면, 가까운 유닛을 향해 이동합니다.
- 샌드웜이 유닛을 발견하는 과정은 동서남북으로 살펴보고 (무한대 시야) 
  유닛을 발견하면 이동하고, 발견하지 못하면 랜덤으로 한칸씩 이동합니다.
- 샌드웜은 암석 (Rock)을 투시하지 못하게 하였습니다.

Bonus 1)
- 샌드웜은 머리(헤드)만 유닛을 공격할 수 있습니다.
- layer0에 장판(plate)이 있는 경우 공격하지 않습니다.
- 유닛을 공격한 경우, 유닛의 자리로 이동하게되고 꼬리가 증가합니다.
- 일정 확률 (10%)로 스파이스를 생성하며, 근처에 이미 스파이스가 있으면 생성하지 않습니다.
- 샌드웜을 쉽게 구분하기 위하여 머리와 꼬리의 색을 다르게 했습니다.

Bonus 2)
- 사막독수리는 한마리 생성된 후 소멸없이 계속 배회합니다. add_eagle을 통하여 추가 될 수 있습니다.
- 사막태풍은 50초 단위로 생성하며, 생성 후 10초동안 지속됩니다.
- 사막태풍은 정해진 항상 동일한 장소에서 생성됩니다.
- 사막태풍은 유닛이 부딪히면, 유닛이 소멸되고 건물의 경우 내구성이 1/2로 감소합니다.
- 정수로 내구성을 표시하였기 때문에 내구성이 0이 되면 삭제됩니다.

Bonus )
- 본진(b)에서 유닛을 생산하는데 시간이 걸립니다. 생산비용 * 100ms (10 * TICK)
- 명령이 진행되고 있는 동안에는 다른 명령을 수행할 수 없습니다.
- 각 유닛마다 명령들을 수행할 수 있도록 처음에는 명령만 전달하고, 일괄적으로 명령이 있는 유닛들에게 명령 준비 시간을 감소시킵니다.
- 명령 준비 시간이 소진되면 그때 명령을 수행합니다.
- 명령 취소는 'x' 또는 ESC를 이용합니다.