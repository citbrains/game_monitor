#include "stdio.h"
#include "game_state.h"

GameState::GameState() : m_game_state(0), m_remaining_time(0), m_secondary_time(0), m_score1(0), m_score2(0), f_update_score1(false), f_update_score2(false)
{
}

GameState::~GameState()
{
}

void GameState::setData(const char *in_data, const unsigned int data_len)
{
	const unsigned char *data = reinterpret_cast<const unsigned char *>(in_data);
	constexpr unsigned int packet_size = 688;
	if(data_len != packet_size)
		return;
	decodeData(data + 0);
	decodeTeamInfo(data + 24);
	decodeTeamInfo(data + 24 + 308);
}

void GameState::decodeData(const unsigned char *data)
{
	if(!(data[0] == 'R' && data[1] == 'G' && data[2] == 'm' && data[3] == 'e')) {
		return;
	}
	const unsigned int protcol_version = data[5] << 8 | data[4];
	const unsigned int packet_number = data[6];
	const unsigned int players_per_team = data[7];
	const unsigned int game_type = data[8];
	const   signed int state = data[9];
	const unsigned int first_half = data[10];
	const unsigned int kick_off_team = data[11];
	const unsigned int secondary_state = data[12];
	const unsigned int secondary_state_info = data[16] << 24 | data[15] << 16 | data[14] << 8 | data[13];
	const unsigned int drop_in_team = data[17];
	const   signed int drop_in_time = static_cast<signed short>(data[19] << 8 | data[18]);
	const   signed int secs_remaining = static_cast<signed short>(data[21] << 8 | data[20]);
	const   signed int secondary_time = static_cast<signed short>(data[23] << 8 | data[22]);

	constexpr unsigned int PROTCOL_VERSION = 12;
	if(protcol_version != PROTCOL_VERSION)
		return;
	m_game_state = state;
	m_remaining_time = secs_remaining;
	m_secondary_time = secondary_time;
}

void GameState::decodeTeamInfo(const unsigned char *data)
{
	const unsigned int team_number = data[0];
	const unsigned int team_color = data[1];
	const unsigned int score = data[2];
	const unsigned int penalty_shot = data[3];
	const unsigned int single_shots = data[5] << 8 | data[4];
	const unsigned int coach_sequence = data[6];
	const unsigned char *coach_message = data + 7;
	constexpr unsigned int coach_message_size = 253;
	decodeRobotInfo(data + 7 + coach_message_size);
	constexpr unsigned int max_players = 11;
	for(unsigned int i = 0; i < max_players; i++) {
		constexpr unsigned int sizeof_robot_info = 4;
		decodeRobotInfo(data + 260 + sizeof_robot_info + (sizeof_robot_info * i));
	}

	constexpr unsigned int TEAM_COLOR_BLUE = 0;
	constexpr unsigned int TEAM_COLOR_RED = 1;

	if(team_color == TEAM_COLOR_BLUE && m_score1 != score) {
		f_update_score1 = true;
		m_score1 = score;
	} else if(team_color == TEAM_COLOR_RED && m_score2 != score) {
		f_update_score2 = true;
		m_score2 = score;
	}
}

void GameState::decodeRobotInfo(const unsigned char *data)
{
	const unsigned int penalty = data[0];
	const unsigned int secs_till_unpenalised = data[1];
	const unsigned int yello_card_count = data[2];
	const unsigned int red_card_count = data[3];
}

int GameState::getGameState(void)
{
	return m_game_state;
}

int GameState::getRemainingTime(void)
{
	return m_remaining_time;
}

int GameState::getSecondaryTime(void)
{
	return m_secondary_time;
}

bool GameState::updatedScore1(void)
{
	return f_update_score1;
}

bool GameState::updatedScore2(void)
{
	return f_update_score2;
}

unsigned int GameState::getScore1(void)
{
	f_update_score1 = false;
	return m_score1;
}

unsigned int GameState::getScore2(void)
{
	f_update_score2 = false;
	return m_score2;
}

