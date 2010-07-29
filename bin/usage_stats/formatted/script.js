function hideShow(el) {
	var element = el.parentNode.nextSibling.nextSibling;
	if(element.style.display == 'none') {
		element.style.display = 'block';
		el.value = '-';
	} else {
		element.style.display = 'none';
		el.value = '+';
	}
}

function spoiler(element) {
	if(element.style.display == 'none') {
		element.style.display = 'block';
	} else {
		element.style.display = 'none';
	}
}