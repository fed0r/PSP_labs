function SendData(){
	let array = document.getElementById('arraytext').value;
    let sortedarray = document.getElementById('sortedarray');
	var request = new XMLHttpRequest();
    request.open("POST", '/' + array);

    // Функция обратного вызова, когда запрос завершен
      request.onload = () => {
          sortedarray.innerHTML = request.responseText;
      }

    request.send();
}