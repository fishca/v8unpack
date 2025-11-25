#!/bin/bash

# run_refactoring_tests.sh - Скрипт запуска базовых тестов рефакторинга (Этап 1.2)

set -e  # Выход при ошибке

echo "====================================="
echo "Этап 1: Анализ и подготовка - Тестирование"
echo "====================================="
echo ""

# Функция для логирования результатов
log_result() {
    local test_name=$1
    local success=$2
    local output=$3

    if [ $success -eq 0 ]; then
        echo "✅ $test_name: PASSED"
    else
        echo "❌ $test_name: FAILED"
        if [ -n "$output" ]; then
            echo "   Error output: $output"
        fi
    fi
}

# Проверка наличия необходимых файлов
check_files() {
    local files=("test_application.cpp" "analysis_main_cpp.md" "plan_refactoring.md")

    echo "Проверка наличия файлов анализа..."
    local missing_files=()

    for file in "${files[@]}"; do
        if [ ! -f "$file" ]; then
            missing_files+=("$file")
        fi
    done

    if [ ${#missing_files[@]} -ne 0 ]; then
        echo "❌ Отсутствуют файлы анализа:"
        for file in "${missing_files[@]}"; do
            echo "   - $file"
        done
        return 1
    else
        echo "✅ Все файлы анализа найдены"
        return 0
    fi
}

# Компиляция и запуск тестов
run_basic_tests() {
    echo ""
    echo "Компиляция и запуск базовых тестов..."

    local test_file="test_application.cpp"
    local test_executable="test_application"

    # Проверка что компилятор доступен
    if ! command -v g++ &>/dev/null && ! command -v clang++ &>/dev/null; then
        echo "❌ Компилятор C++ не найден (нужен g++ или clang++)"
        return 1
    fi

    # Выбор компилятора
    local compiler="g++"
    if command -v clang++ &>/dev/null; then
        compiler="clang++"
    fi

    echo "Использование компилятора: $compiler"

    # Компиляция тестов (без GTest - standalone mode)
    echo "Компиляция $test_file..."
    if $compiler -std=c++11 -I. -I../src -o "$test_executable" "$test_file" 2>compile_error.log; then
        echo "✅ Компиляция успешно"
    else
        echo "❌ Ошибка компиляции:"
        cat compile_error.log
        return 1
    fi

    # Запуск тестов
    echo "Запуск тестов..."
    if ./"$test_executable"; then
        log_result "Базовые инфраструктурные тесты" 0
        return 0
    else
        local exit_code=$?
        log_result "Базовые инфраструктурные тесты" 1 "Exit code: $exit_code"
        return 1
    fi
}

# Проверка корректности анализа main.cpp
validate_analysis() {
    echo ""
    echo "Валидация корректности анализа..."

    local analysis_file="analysis_main_cpp.md"
    local required_sections=(
        "Последовательность выполнения main"
        "Диаграмма зависимостей функций"
        "Код метрики"
    )

    local all_sections_found=true

    for section in "${required_sections[@]}"; do
        if ! grep -q "$section" "$analysis_file"; then
            echo "❌ Отсутствует секция: $section"
            all_sections_found=false
        fi
    done

    if [ "$all_sections_found" = true ]; then
        echo "✅ Все необходимые секции анализа найдены"

        # Дополнительная проверка качества анализа
        local line_count=$(wc -l < "$analysis_file")
        if [ $line_count -gt 50 ]; then
            echo "✅ Анализ достаточно подробный ($line_count строк)"
            return 0
        else
            echo "⚠️  Анализ может быть недостаточно подробный ($line_count строк)"
            return 0
        fi
    else
        return 1
    fi
}

# Проверка плана рефакторинга
validate_refactoring_plan() {
    echo ""
    echo "Валидация плана рефакторинга..."

    local plan_file="plan_refactoring.md"
    local required_patterns=(
        "Этапы реализации"
        "Риски и Mitigation"
        "Критерии успеха"
        "Общий timeline"
    )

    local plan_complete=true

    for pattern in "${required_patterns[@]}"; do
        if ! grep -q "$pattern" "$plan_file"; then
            echo "❌ Отсутствует: $pattern"
            plan_complete=false
        fi
    done

    if [ "$plan_complete" = true ]; then
        echo "✅ План рефакторинга содержит все необходимые разделы"

        # Проверка структуры новой архитектуры
        if grep -q "src/app/" "$plan_file" && grep -q "src/metadata/" "$plan_file"; then
            echo "✅ Структура новой архитектуры описана"
            return 0
        else
            echo "⚠️  Структура архитектуры может быть неполной"
            return 0
        fi
    else
        return 1
    fi
}

# Основная функция
main() {
    echo "Начало тестирования этапа анализа и подготовки..."
    echo "Timestamp: $(date)"
    echo ""

    local overall_success=true

    # 1. Проверка файлов анализа
    if ! check_files; then
        overall_success=false
    fi

    # 2. Валидация анализа
    if ! validate_analysis; then
        overall_success=false
    fi

    # 3. Валидация плана рефакторинга
    if ! validate_refactoring_plan; then
        overall_success=false
    fi

    # 4. Запуск базовых тестов
    if ! run_basic_tests; then
        overall_success=false
    fi

    echo ""
    echo "====================================="

    if [ "$overall_success" = true ]; then
        echo "🎉 ЭТАП АНАЛИЗА И ПОДГОТОВКИ ЗАВЕРШЕН УСПЕШНО!"
        echo ""
        echo "Следующие шаги:"
        echo "1. ✓ Анализ завершен - перейти к разработке спецификаций классов"
        echo "2. Начать реализацию утилитарных классов (StringConverters, ErrorCodes)"
        echo "3. Разрабатывать тесты параллельно с кодом"
        echo ""
        echo "Рекомендуется:"
        echo "- Показать результаты команде для одобрения"
        echo "- Начать спецификации для ключевых классов (Application, ArgumentParser)"
        exit 0
    else
        echo "❌ ЭТАП АНАЛИЗА И ПОДГОТОВКИ НЕ ЗАВЕРШЕН"
        echo ""
        echo "Необходимо исправить выявленные проблемы перед продолжением."
        echo "Проверьте логи выше для деталей ошибок."
        exit 1
    fi
}

# Запуск основной функции
main "$@"
