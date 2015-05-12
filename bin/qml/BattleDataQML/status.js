var statusArray = {};

statusArray[PokeData.Poisoned] = " {PSN}";
statusArray[PokeData.Koed] = " {KO]";
statusArray[PokeData.Frozen] = " [FRZ]";
statusArray[PokeData.Asleep] = " [SLP]";
statusArray[PokeData.Burnt] = " [BRN]";
statusArray[PokeData.Paralysed] = " [PAR]";
statusArray[PokeData.Fine] = ""

function statusName(status) {
    return statusArray[status];
}
