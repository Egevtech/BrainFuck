# BrainFuck compiler [EN/[RU](#RU)]
This repository contains my implementation of brainfuck compiler

> [!WARNING]
> This project currently in development stage

## Project status
- [X] Compiling brainfuck code into assembler 
- [X] Building executable file
- [ ] Loop instructions
- [ ] Input instructions
- [ ] Some custom instructions

## FULL compatibility with the original project is planned.
Now, there are basic instructions from the BrainFuck:
- `>` - select next cell
- `<` - select prev cell
- `+` - add one to selected cell
- `-` - subtract one from selected cell
- `.` - print data in cell as char

But, there are also some custom ones:
- `p` - print number in the current cell
- `P` - similar to `p`, but with `\n` added to the end

<a name="RU"></a>
# Компилятор BrainFuck
В этом репозитории находится моя версия компилятора языка программирования BrainFuck

> [!WARNING]
> Проект находится в стадии разработки, возможны ошибки и недочеты

## Статус проекта
- [X] Компиляция кода на BrainFuck в ассемблер
- [X] Сборка исполняемого файла
- [ ] Инструкции цикла
- [ ] Инструкции ввода
- [ ] Несколько кастомных инструкций

## В планах обеспечить ПОЛНУЮ совместимость с оригинальным проектом
Сейчас компилятор поддерживает базовые инструкции из BrainFuck:
- `>` - выбрать следующую ячейку
- `<` - выбрать предыдущую ячейку
- `+` - добавить еденицу к выбранной ячейке
- `-` - вычесть еденицу из выбранной ячейки
- `.` - напечатать содержимое ячейки в символьном представлении

Но есть и несколько добавленных мной инструкций
- `p` - Напечатать число из выбранной ячейки
- `P` - Напечатать число из выбранной ячейки, но добавить перенос строки в конец