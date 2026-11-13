CC=gcc
CFLAGS=-g -Wall -I./app
# 생성될 폴더들의 이름을 변수로 지정
OBJ_DIR=obj
BIN_DIR=bin

# 타겟과 오브젝트 파일들에 폴더 경로를 추가
TARGET=$(BIN_DIR)/app.out
OBJS=$(OBJ_DIR)/main.o \
	$(OBJ_DIR)/print_menu.o \
	$(OBJ_DIR)/run_ls.o \
	$(OBJ_DIR)/run_cd.o \
	$(OBJ_DIR)/run_cp.o \

# 1. 최종 실행 파일 생성
all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)	# bin 폴더가 없으면 생성 
	$(CC) -o $(TARGET) $(OBJS) -lncursesw

# 2. 오브젝트 파일들을 obj 폴더에 생성 
# -o 옵션으로 저장 폴더 지정
$(OBJ_DIR)/main.o: app/main.h app/main.c
	@mkdir -p $(OBJ_DIR)	# obj 폴더가 없으면 생성
	$(CC) $(CFLAGS) -c app/main.c -o $(OBJ_DIR)/main.o -lncursesw

$(OBJ_DIR)/print_menu.o: app/main.h lib/print_menu.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c lib/print_menu.c -o $(OBJ_DIR)/print_menu.o -lncursesw

$(OBJ_DIR)/run_ls.o: app/main.h lib/run_ls.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c lib/run_ls.c -o $(OBJ_DIR)/run_ls.o -lncursesw


$(OBJ_DIR)/run_cd.o: app/main.h lib/run_cd.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c lib/run_cd.c -o $(OBJ_DIR)/run_cd.o -lncursesw


$(OBJ_DIR)/run_cp.o: app/main.h lib/run_cp.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c lib/run_cp.c -o $(OBJ_DIR)/run_cp.o -lncursesw

# 3. clean 작업: 폴더를 삭제함 
# (-r 옵션: 폴더 내부까지 삭제)
.PHONY: clean
clean:
	@echo "Cleaning obj, bin folders"
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Clean complete"
