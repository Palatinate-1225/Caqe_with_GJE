# 指向 src 目錄
CMS_INCLUDE_DIR = /cryptominisat/src

# 確保編譯參數包含這個路徑
CXXFLAGS += -I$(CMS_INCLUDE_DIR)
# --- 編譯參數 ---
CXX = g++
# 記得要 c++17，因為你的 qbf.cpp 有用到結構化綁定
CXXFLAGS = -std=c++17 -Wall -O3

# 在 CXXFLAGS 中加入 -I (Include) 參數
CXXFLAGS += -I$(CMS_INCLUDE_DIR)

# 在 LDFLAGS 中加入 -L (Library Path) 與 -l (Library Name)
LDFLAGS = -L$(CMS_LIB_DIR) -lcryptominisat5 -lz

# --- 目標與規則 ---
TARGET = qbf_solver
OBJS = main.o qbf.o sat.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

# 編譯 sat.o 時，編譯器會根據 CXXFLAGS 中的 -I 路徑去找 cryptominisat.h
sat.o: sat.cpp sat.h
	$(CXX) $(CXXFLAGS) -c sat.cpp

# ... 其餘規則保持不變 ...