var colorArray = {};

colorArray[PokeData.Poisoned] = "#00a040a0";
colorArray[PokeData.Koed] = "#00000000";
colorArray[PokeData.Frozen] = "#0098d8d8";
colorArray[PokeData.Asleep] = "#00f85888";
colorArray[PokeData.Burnt] = "#00f08030";
colorArray[PokeData.Paralysed] = "#00f8d030";
colorArray[PokeData.Fine] = "#00ffffff"

function statusColor(status) {
    return colorArray[status];
}
