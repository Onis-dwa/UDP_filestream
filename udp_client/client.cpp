#include <iostream>
#include <random>
#include <thread>
#include "client.h"

#include "common.hpp"

using namespace std;

// рандомайзер
static random_device rd;
static mt19937 mersenne(rd());

client::client():
	_socket(),

	_packetsInfo({}),
	_packsCount(0),
	_rawData(nullptr),
	_totalLen(0),
	_backLen(0),
	_packsLeft(0),
	_checksummCorrect(false)
{}

udp_socket::status client::connectTo(const string& addr, const uint16_t port) {
	return _socket.init(addr.c_str(), port);
}

int client::exec() {
#ifdef CLIENT_LOOP
	while (true)
#endif
	{
		// генерация размера псевдофайла
		_packsCount = mersenne() % MAX_PACKS + 1;
		_backLen = mersenne() % (data_size - 1) + 1; // минимум 1 байт
		_totalLen = (_packsCount - 1) * data_size + _backLen;
		cout << "New file with packet count: "
			<< _packsCount << " " << _totalLen << endl;

		// обнуление
		_pd.id = 0;
		_pd.type = packetType::PUT;
		_pd.seq_total = _packsCount;
		_packetsInfo.resize(_packsCount);
		_packsLeft = _packsCount;
		_checksummCorrect = false;

		// заполнение псевдофайла
		for (auto& packet : _packetsInfo)
			packet = packetType::PUT;
		if (_rawData) ::operator delete(_rawData, _totalLen);
		_rawData = new uint8_t[_totalLen];
		for (uint32_t i = 0; i < _totalLen; ++i)
			//_rawData[i] = mersenne() % 256;
			_rawData[i] = mersenne() % 94 + 33;

#ifdef PACK_INFO_PRINT
		prntArr(_rawData, _packsCount, _backLen);
#endif
		
		// получение id
		if (getId()) return 1;
		// отправка оставшегося
		if (sendAll()) return 2;
		if (!_checksummCorrect) return 3;
	}
	return 0;
}

uint32_t client::getNext() {
	uint32_t indx = mersenne() % _packsLeft;
	while (indx < _packsCount && _packetsInfo[indx] == packetType::ACK) { ++indx; };
	return indx;
}
bool client::getId() {
	uint32_t indx = getNext();
	uint32_t size = (indx == (_packsCount - 1) ? _backLen : data_size);
	_pd.seq_number = indx;
	deepCopyPD(&_rawData[indx * data_size], &_pd.data[0], size);
	
	int cnt = 5; // количество попыток
	while (_pd.id == 0 && cnt > 0) {
		// отправляем наш пакет
		if (_socket.udp_send(_pd, size + 17) <= 0) { // +17 packetData
			cout << "err send: " << udp_socket::getError() << " i: " << indx << endl;
			return true;
		}

		// должны получить ответ в течении 2-х секунд
		auto begin = steady_clock::now();
		while (duration_cast<seconds>(steady_clock::now() - begin).count() < 2) {
			auto pack = _socket.udp_recv();
			if (pack.first.dataSize > 16) {
				// небольшой гуард
				if (pack.second->type != packetType::ACK) continue;
				if (pack.second->seq_number > pack.second->seq_total) continue;

#ifdef PACK_INFO_PRINT
				cout << "from: " << pack.first.address
					<< ":" << ntohs(pack.first.port)
					<< " i: " << strAlign(pack.second->seq_number).substr(5, 5)
					<< " s: " << strAlign(pack.first.dataSize).substr(6, 4)
					<< " b: ";
				prt(_pd.data, size);
				if (size > 4) prt(&_pd.data[size - 4]);
				cout << " id: " << pack.second->id << endl;
#endif
				if (pack.second->id != 0) {
					_pd.id = pack.second->id;
					_packetsInfo[indx] = packetType::ACK; // не забываем засеттить что пакет был получен
					--_packsLeft;
					
					if (pack.first.dataSize - 17 > 0) // проверим, если пакет единственный
						_checksummCorrect = checkCheckSumm(&pack.second->data[0], pack.first.dataSize - 17);
					return false;
				}
			}
			
			this_thread::sleep_until(steady_clock::now()+ microseconds(50));
		}
		--cnt;
	}
	return true;
}
bool client::sendAll() {
	if (_checksummCorrect) return false;

	while (_packsLeft > 0) {
		uint32_t indx = getNext(); // рандомим блок
		uint32_t size = (indx == (_packsCount - 1) ? _backLen : data_size);
		_pd.seq_number = indx;
		deepCopyPD(&_rawData[indx * data_size], &_pd.data[0], size);

		// пушим блок
		if (_socket.udp_send(_pd, size + 17) <= 0) { // +17 packetData
			cout << "err send: " << udp_socket::getError() << " i: " << indx << endl;
			return true;
		}

		// читаем если есть (если нечего читать, моментально -1)
		auto pack = _socket.udp_recv();
		if (pack.first.dataSize > 16) {
			// небольшой гуард
			if (pack.second->type != packetType::ACK) continue;
			if (pack.second->seq_number > pack.second->seq_total) continue;

#ifdef PACK_INFO_PRINT
			cout << "from: " << pack.first.address
				<< ":" << ntohs(pack.first.port)
				<< " i: " << strAlign(pack.second->seq_number).substr(5, 5)
				<< " s: " << strAlign(pack.first.dataSize).substr(6, 4)
				<< " b: ";
			prt(_pd.data, size);
			if (size > 4) prt(&_pd.data[size - 4]);
#endif
			if (pack.second->id == _pd.id) {
				_packetsInfo[pack.second->seq_number] = packetType::ACK; // сеттим что пакет получен
				--_packsLeft;
				
				if (pack.first.dataSize - 17 > 0) // проверяем если есть crc
					_checksummCorrect = checkCheckSumm(&pack.second->data[0], pack.first.dataSize - 17);
			}
#ifdef PACK_INFO_PRINT
			else
				cout << " id isn't equal!";
			cout << endl;
#endif
		}
	}
	
	return false;
}
bool client::checkCheckSumm(const uint8_t* data, const int size) {
	if (_packsLeft > 0) return false;
	if (size != 4) return false;
	
	uint32_t mcrc = crc32c(0, _rawData, _totalLen);
	uint32_to_char rcrc;
	rcrc.chars[0] = data[0];
	rcrc.chars[1] = data[1];
	rcrc.chars[2] = data[2];
	rcrc.chars[3] = data[3];

#ifdef PACK_INFO_PRINT
	cout << endl
		<< "mcrc: " << mcrc << endl
		<< "rcrc: " << rcrc.value << endl;
#endif
	return rcrc.value == mcrc;
}
