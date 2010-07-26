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