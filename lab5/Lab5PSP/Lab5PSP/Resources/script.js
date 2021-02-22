function SendData(){
	let array = document.getElementById('arraytext').value;
	let sortarray = document.getElementById('sortarray');
	var request = new XMLHttpRequest();
    request.open("POST", '/' + array);

    // Функция обратного вызова, когда запрос завершен
      request.onload = () => {
          sortarray.innerHTML = request.responseText;
      }

    request.send();
}