# Основные настройки:
# CHIP = 				STM32F10X_MD 				# Семейство микроконтроллера
# HSE_VALUE = 			8000000 					# Частота иточника внешнего тактирования микроконтроллера
STONE = 				STM32F103CB 				# Серия микроконтроллера
MCU = 					cortex-m3 					# Ядро микроконтроллера
LD_SCRIPT =				FLASH.ld					# Имя файла скрипта для линковщика
OPTIMIZATION_LEVEL = 	0							# Уровень оптимизации компилятора

# Папки с исходными файлами проекта:
SRCDIR =  $(WORKSPACE_ROOT)/
SRCDIR += $(WORKSPACE_ROOT)/Libraries/CMSIS/
SRCDIR += $(WORKSPACE_ROOT)/Libraries/SPL/
SRCDIR += $(WORKSPACE_ROOT)/User/inc/
SRCDIR += $(WORKSPACE_ROOT)/User/src/

# Директивы препроцессора:
# DEFS = -D$(CHIP)
# DEFS += -DHSE_VALUE=$(HSE_VALUE)
# DEFS += -DUSE_STDPERIPH_DRIVER
# DEFS += -DUSE_FULL_ASSERT
# DEFS += -DVER_MAJOR=$(VER_MAJOR)
# DEFS += -DVER_MINOR=$(VER_MINOR)

# Набор программ для компиляции:
CC = arm-none-eabi-gcc 								# Для компиляции *.c
CPP = arm-none-eabi-g++								# Для компиляции *.cpp
AS = arm-none-eabi-gcc								# Для компиляции *.s
LD = arm-none-eabi-gcc								# Линковщик
OBJCOPY = arm-none-eabi-objcopy						# Для копирования содержимого объектного файла в другой объектный файл
OBJDUMP = arm-none-eabi-objdump						# Для отображения информации о объектном файле
SIZE = arm-none-eabi-size -d						# Для подсчёта размера объектного файла

# Утилита для загрузки прошивки в микроконтроллер:
FLASHER = JLink.exe 								# Используемая утилита
JLINK_PARAMS_LOAD =  -device $(STONE)				# Серия микроконтроллера
JLINK_PARAMS_LOAD += -if SWD 						# Интерфейс
JLINK_PARAMS_LOAD += -speed 4000 					# Cкорость
JLINK_PARAMS_LOAD += -commanderScript FLASH.jlink 	# Имя файла скрипта

# Определение порядка расположения файлов и папок:
WORKSPACE_ROOT = .
TARGET = firmware
INCDIR = $(patsubst %,-I "%",$(SRCDIR))
OUTDIR = $(WORKSPACE_ROOT)/Output
OBJDIR = $(WORKSPACE_ROOT)/$(OUTDIR)/obj
EXEDIR = $(WORKSPACE_ROOT)/$(OUTDIR)/frw
LSTDIR = $(WORKSPACE_ROOT)/$(OUTDIR)/lst
HEX = $(EXEDIR)/$(TARGET).hex
BIN = $(EXEDIR)/$(TARGET).bin
ELF = $(EXEDIR)/$(TARGET).elf
MAP = $(LSTDIR)/$(TARGET).map
LSS = $(LSTDIR)/$(TARGET).lss

# Перечень файлов, которые будут передаваться линковщику:
CSRC = $(wildcard $(addsuffix /*.c,$(SRCDIR)))
CPPSRC = $(wildcard $(addsuffix /*.cpp,$(SRCDIR)))
ASRC = $(wildcard $(addsuffix /*.s,$(SRCDIR)))
OBJS = $(addprefix $(OBJDIR)/,$(notdir $(CSRC:.c=.o) $(CPPSRC:.cpp=.o) $(ASRC:.s=.o) ))



# Определеяем флаги для компиляции и линковки
# Общие флаги

# Название ядра:
FLAGS = -mcpu=$(MCU)

# Система команд ядра микроконтроллера:
FLAGS += -mthumb

# Директивы препроцессора:
FLAGS += $(DEFS)

# Директории в которых находятся дополнительные исходные файлы проекта
# (сюда передается в виде "-I ."):
FLAGS += $(INCDIR)

# "-Wa" - передает все последующие флаги в "arm-none-eabi-as"
# "-adhlns" - флаг, для созданя ассемблерного листинга
FLAGS += -Wa,-adhlns=$(addprefix $(LSTDIR)/, $(notdir $(addsuffix .lst,$(basename $<))))

# Информация о зависимостях записывается в файл,
# получающийся при замене ".c" на ".d" на концах имен входных файлов:
FLAGS += -MD

# Флаги для *.c файлов:

# Включаем сюда общие флаги тоже:
CFLAGS = $(FLAGS)

# Общий уровень оптимизации:
CFLAGS += -O$(OPTIMIZATION_LEVEL)

# Включаем отладочную информацию в объектный файл для GDB
# (это позволит отладчику GDB, дать подробную информацию о процессе отладки):
CFLAGS += -g -gdwarf-2

# Указываем gcc использовать каналы в памяти вместо временных файлов для
# обмена данными между разными стадиями компиляции:
#CFLAGS += -pipe

# Разбиваем функции и данные в две разные секции:
CFLAGS += -ffunction-sections -fdata-sections

# Включаем вывод дополнительных предупреждений при компиляции проекта:
CFLAGS += -Wextra -Wundef -Wcast-align -Winline -Wall -Wno-missing-braces

# Флаги для математического ядра микроконтроллера:
#CFLAGS += -mfpu=fpv4-sp-d16
#CFLAGS += -mfloat-abi=hard

# Флаги для *.cpp файлов:

# Включаем сюда общие флаги, и флаги для *.c файлов:
CPPFLAGS = $(CFLAGS)

# Включаем стандарт Си++11 с расширениями GNU:
CPPFLAGS += -std=gnu++0x

# Отключаем исключения:
CPPFLAGS += -fno-exceptions

# Флаг для компиляции С++ без runtime type information:
CPPFLAGS += -fno-rtti

# Заменять любой неквалифицированный тип
# битовых полей (unqualified bitfield type) на беззнаковый (unsigned):
CPPFLAGS += -funsigned-bitfields

# Выделям перечисленному типу (enum) только такое количество байт,
# сколько требуется для объявленного:
CPPFLAGS += -fshort-enums

# Флаги для *.s файлов:
AFLAGS = $(CFLAGS)

# Для компиляции ассемблер-файлов использовать arm-none-eabi-gcc:
AFLAGS += -x assembler-with-cpp

# Флаги для линовщика:

# Название ядра:
LD_FLAGS = -mcpu=$(MCU)

# Система команд ядра микроконтроллера:
LD_FLAGS += -mthumb

# "-Wl" - передает все последующие флаги линковщику,
# "-Map" - флаг, для создания файла .map,
# "--cref" - для генерации таблицы перекрестных ссылок
LD_FLAGS += -Wl,-Map="$(MAP)",--cref

# Удаляем неиспользуемые секции:
LD_FLAGS += -Wl,--gc-sections

#LD_FLAGS += -Wl,--start-group
#LD_FLAGS += -Wl,-lnosys

# Имя файла скрипта для линковки:
LD_FLAGS += -T$(LD_SCRIPT)

# Не передавать в линковщик стандартный стартап файл от GCC:
LD_FLAGS += -nostartfiles

# Передаем компилятору все пути к папкам, содержащим .c, .cpp, .s файлы проекта:
vpath %.c $(SRCDIR)
vpath %.cpp $(SRCDIR)
vpath %.s $(SRCDIR)

.PHONY: clean dirs program
.SILENT:

# Список целей при построении проекта:
build_and_program:
	$(MAKE) build
	$(MAKE) program

rebuild_and_program:
	$(MAKE) rebuild
	$(MAKE) program

build:
	@echo Building...
	rm -f $(OBJDIR)/main.o
	$(MAKE) $(ELF)
	$(MAKE) $(BIN)
	$(MAKE) $(LSS)
	$(MAKE) $(HEX)
	$(SIZE) $(ELF)
	@echo Done!

rebuild:
	@echo Rebuilding...
	$(MAKE) clean
	$(MAKE) build

clean:
	@echo Cleaning output...
	rm -f $(OBJDIR)/*
	rm -f $(LSTDIR)/*
	rm -f $(EXEDIR)/*
	@echo Done!

dirs:
	@echo Creating dirs...
	mkdir $(OUTDIR)
	mkdir $(OBJDIR)
	mkdir $(LSTDIR)
	mkdir $(EXEDIR)
	@echo Done!

program:
	@echo Programming with $(FLASHER)...
	$(FLASHER) $(JLINK_PARAMS_LOAD)
	@echo Done!

$(ELF): $(OBJS) makefile
	@echo Linking...
	@echo Making elf...
	# "-o" - флаг, после которого задается имя файла, который будет получен
	$(LD) $(OBJS) $(LD_FLAGS) -o $(ELF)

$(HEX): $(ELF) makefile
	@echo Making hex...
	$(OBJCOPY) -O ihex $(ELF) $(HEX)

$(BIN): $(ELF) makefile
	@echo Making bin...
	$(OBJCOPY) -O binary $(ELF) $(BIN)

$(LSS): $(ELF) makefile
	@echo Making lss...
	$(OBJDUMP) -dC $(ELF) > $(LSS)


$(OBJDIR)/%.o: %.cpp makefile
	@echo Compiling $<...
	# "-o" - флаг, после которого задается имя файла, который будет получен
	# "-c" - скомпилировать исходные файлы без линковки
	$(CPP) -c $(CPPFLAGS) -o $@ $<

$(OBJDIR)/%.o: %.c makefile
	@echo Compiling $<...
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJDIR)/%.o: %.s makefile
	@echo Assembling $<...
	$(AS) -c $(AFLAGS) -o $@ $<
