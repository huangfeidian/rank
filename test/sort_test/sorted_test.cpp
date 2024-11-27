#include "array_rank.h"
#include <random>
using namespace spiritsaway::system::rank;

void test_1()
{
	std::uint32_t rank_num = 100;
	std::uint32_t pool_num = 200;
	std::vector<rank_info> temp_ranks;
	for (int i = 0; i < 2 * pool_num; i++)
	{
		rank_info cur_rank_info;
		cur_rank_info.player_id = "player_" + std::to_string(i);
		cur_rank_info.rank_value = -1.0 * i;
		cur_rank_info.update_ts = 2*pool_num - i;
		temp_ranks.push_back(cur_rank_info);
	}
	std::random_device r;
	std::default_random_engine e1(r());
	for (int i = 0; i < 10; i++)
	{
		auto cur_array_rank = array_rank("temp", rank_num, pool_num);
		std::shuffle(temp_ranks.begin(), temp_ranks.end(), e1);
		for (const auto& one_info : temp_ranks)
		{
			cur_array_rank.update(one_info);
		}
		for (int j = 0; j < rank_num; j++)
		{
			auto cur_player_id = "player_" + std::to_string(j);
			auto cur_player_rank = cur_array_rank.get_rank(cur_player_id);
			assert(cur_player_rank  == (j + 1));
		}
	}
}


void test_2()
{
	std::uint32_t rank_num = 8;
	std::uint32_t pool_num = 10;
	std::vector<rank_info> temp_ranks;
	for (int i = 0; i <  pool_num; i++)
	{
		rank_info cur_rank_info;
		cur_rank_info.player_id = "player_" + std::to_string(i);
		cur_rank_info.rank_value = 1.0;
		cur_rank_info.update_ts = 1;
		temp_ranks.push_back(cur_rank_info);
	}
	std::random_device r;
	std::default_random_engine e1(r());
	for (int i = 0; i < 10; i++)
	{
		auto cur_array_rank = array_rank("temp", rank_num, pool_num);
		std::shuffle(temp_ranks.begin(), temp_ranks.end(), e1);
		for (const auto& one_info : temp_ranks)
		{
			cur_array_rank.update(one_info);
		}
		for (int j = 0; j < rank_num; j++)
		{
			auto cur_player_id = "player_" + std::to_string(j);
			auto cur_player_rank = cur_array_rank.get_rank(cur_player_id);
			assert(cur_player_rank == (j+1));
		}
	}
}
void test_3()
{
	std::uint32_t rank_num = 100;
	std::uint32_t pool_num = 200;
	std::random_device r;
	std::default_random_engine e1(r());
	std::uniform_int_distribution<int> int_dis(1, 100);

	std::vector<rank_info> temp_ranks;
	for (int i = 0; i < 2 * pool_num; i++)
	{
		rank_info cur_rank_info;
		cur_rank_info.player_id = "player_" + std::to_string(i);
		cur_rank_info.rank_value = int_dis(e1);
		cur_rank_info.update_ts = int_dis(e1)/10;
		temp_ranks.push_back(cur_rank_info);
	}
	
	auto cur_array_rank = array_rank("temp", rank_num, pool_num);
	for (auto one_data : temp_ranks)
	{
		cur_array_rank.update(one_data);
	}
	std::sort(temp_ranks.begin(), temp_ranks.end());
	for (int i = 0; i < temp_ranks.size(); i++)
	{
		auto cur_player_rank = cur_array_rank.get_rank(temp_ranks[i].player_id);
		if (i >= cur_array_rank.size() || i >= cur_array_rank.m_rank_sz)
		{
			assert(!cur_player_rank);
		}
		else
		{
			assert(cur_player_rank == (i+1));
		}
	}
	std::shuffle(temp_ranks.begin(), temp_ranks.end(), e1);
	for (int i = 0; i < pool_num; i++)
	{
		auto cur_back = temp_ranks.back();
		cur_array_rank.remove(cur_back.player_id);
		temp_ranks.pop_back();
	}
	std::sort(temp_ranks.begin(), temp_ranks.end());
	for (int i = 0; i < temp_ranks.size(); i++)
	{
		auto cur_player_rank = cur_array_rank.get_rank(temp_ranks[i].player_id);
		if (i >= cur_array_rank.size() || i >= cur_array_rank.m_rank_sz)
		{
			assert(!cur_player_rank);
		}
		else
		{
			assert(cur_player_rank == (i+1));
		}
	}
}

int main()
{
	test_1();
	test_2();
	test_3();
	return 1;
}