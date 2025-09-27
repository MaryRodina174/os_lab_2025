if [ $# -eq 0 ]; then
    echo "Ошибка: укажите хотя бы одно число как аргумент."
    exit 1
fi

sum=0
count=0

# Перебор аргуменотв
for num in "$@"; do
    # Яыляется ли аргумент числом?
    if ! [[ "$num" =~ ^-?[0-9]+([.][0-9]+)?$ ]]; then
        echo "Ошибка: '$num' не является числом."
        exit 1
    fi
    sum=$(echo "$sum + $num" | bc)
    ((count++))
done

average=$(echo "scale=2; $sum / $count" | bc)

echo "Количество аргументов: $count"
echo "Среднее арифметическое: $average"
