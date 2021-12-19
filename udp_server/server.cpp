#include <iostream>
#include <random>
#include "server.h"

#include "common.hpp"

// рандомайзер
static random_device rd;
static mt19937_64 mersenne(rd());

server::server():
	_socket()
{}

udp_socket::status server::start(const string & addr, const uint16_t port) {
	if (_socket.init(addr.c_str(), port) != udp_socket::status::init)
		return udp_socket::status::init;
	return _socket.start();
}

uint16_t server::getServerPort() const {
	return _socket.getPort();
}

int server::exec() {
	lastExpiredCheck = steady_clock::now();
	while (true) {
		auto p = _socket.udp_recv();
		auto currTime = steady_clock::now();

		// нас интересуют только успешные запросы
		if (p.first.dataSize > 17) {
			// небольшой контроль ошибок
			if (p.second->type != packetType::PUT) continue;
			if (p.second->seq_number > p.second->seq_total) continue;

			// если id нету сгенерируем
			if (p.second->id == 0) {
				p.second->id = mersenne();
#if CXX_VERSION == 20
				while (_packets.contains(p.second->id)) p.second->id = mersenne();
#else
				for (auto it = _packets.find(p.second->id);
					 it != _packets.end();
					 it = _packets.find(p.second->id))
					p.second->id = mersenne();
#endif
				cout << "New client: " << p.first.address
					<< ":" << ntohs(p.first.port)
					<< " id: " << p.second->id
					<< " cnt: " << p.second->seq_total
					<< " expected size: " << (p.second->seq_total * data_size)
					<< endl;

				// создаём запись о файле
				_packets[p.second->id] = {
					currTime,
					0,
					p.second->seq_total,
					(p.second->seq_total-1) * data_size,
					0,
					new uint8_t[p.second->seq_total * data_size]
				};
			}

			// сохраняем данные
			_packets[p.second->id].lastActive = currTime;
			++_packets[p.second->id].seq_current;
			deepCopyPD(
				&p.second->data[0],
				&_packets[p.second->id].data[p.second->seq_number * data_size],
				p.first.dataSize - 17
			);

			// подсчитываем размер хвоста
			if (p.second->seq_number == (p.second->seq_total-1)) {
				_packets[p.second->id].len_back   = p.first.dataSize - 17;
				_packets[p.second->id].len_total += p.first.dataSize - 17;
			}

			// отправляем ACK
#ifdef PACK_INFO_PRINT
			cout << "id: " << p.second->id
				<< " i: " << strAlign(p.second->seq_number).substr(5, 5)
				<< " s: " << strAlign(p.first.dataSize).substr(6, 4)
				<< " d: ";
			prt(p.second->data, p.first.dataSize - 21);
			if (p.first.dataSize - 21 > 4) prt(&p.second->data[p.first.dataSize - 21]);
#endif
			p.second->type = packetType::ACK;
			if (_packets[p.second->id].seq_current == _packets[p.second->id].seq_total) {
				// crc и завершаем соединение
				uint32_to_char crc;
				crc.value = crc32c(0, _packets[p.second->id].data, _packets[p.second->id].len_total);
#ifdef PACK_INFO_PRINT
				cout << " crc32: " <<  crc.value << " s: " << _packets[p.second->id].len_total << endl;
#endif
				p.first.dataSize = 21;
				p.second->data[0] = crc.chars[0];
				p.second->data[1] = crc.chars[1];
				p.second->data[2] = crc.chars[2];
				p.second->data[3] = crc.chars[3];

#ifdef PACK_INFO_PRINT
				prntArr(
					_packets[p.second->id].data,
					_packets[p.second->id].seq_total,
					_packets[p.second->id].len_back
				);
#endif
				_socket.udp_sendC(*p.second, p.first);
				::operator delete(_packets[p.second->id].data, _packets[p.second->id].seq_total * data_size);
				_packets.erase(p.second->id);
			}
			else {
				// отправляем что блок получили
#ifdef PACK_INFO_PRINT
				cout << endl;
#endif
				p.first.dataSize = 17;
				_socket.udp_sendC(*p.second, p.first);
			}
		}

		// раз в 40с чекаем устаревшие сокеты
		if (duration_cast<seconds>(currTime - lastExpiredCheck).count() > 40) {
			vector<decltype(_packets.begin())> expiredConnections;
			for (auto it = _packets.begin(); it != _packets.end(); ++it) {
				if (duration_cast<seconds>(currTime - (*it).second.lastActive).count() > 40) {
					expiredConnections.push_back(it);
				}
			}

			for (const auto& it : expiredConnections) {
				::operator delete((*it).second.data, (*it).second.seq_total * data_size);
				_packets.erase(it);
			}

			lastExpiredCheck = currTime;
		}
	}
	return 0;
}
