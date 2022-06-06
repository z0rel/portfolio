
const setupSvgLocalstorage = (window, document, file, prefix) => {
	'use strict';

	let revision = 8;

	if (!document.createElementNS || !document.createElementNS( 'http://www.w3.org/2000/svg', 'svg' ).createSVGRect) {
		return true;
	}

	let isLocalStorage = 'localStorage' in window && window[ 'localStorage' ] !== null;
	let	request;
	let	data;

	let	insertIT = () => {
		document.body.insertAdjacentHTML( 'afterbegin', data );
	}

	let	insert = () => {
		if (document.body) {
			insertIT();
		}
		else {
			document.addEventListener('DOMContentLoaded', insertIT);
		}
	};

	if (isLocalStorage && localStorage.getItem( 'inlineSVGrev' + prefix ) == revision) {
		data = localStorage.getItem( 'inlineSVGdata' + prefix );
		if (data) {
			insert();
			return true;
		}
	}

	try {
		request = new XMLHttpRequest();
		request.open( 'GET', file, true );
		request.onload = () => {
			if (request.status >= 200 && request.status < 400) {
				data = request.responseText;
				insert();
				if (isLocalStorage) {
					localStorage.setItem('inlineSVGdata' + prefix,  data);
					localStorage.setItem('inlineSVGrev' + prefix, revision);
				}
			}
		}
		request.send();
	}
	catch (e) {

	}
};

export default setupSvgLocalstorage;
 

// {% raw %}
// setup_svg_localstorage(window, document, '/img/{{ include.filename }}', '{{ include.tag }}');
// {% endraw %}


