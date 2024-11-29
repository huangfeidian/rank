#include "array_rank.h"
#include "skiplist_rank.h"

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
		temp_ranks.push_back(cur_rank_info);
	}
	std::random_device r;
	std::default_random_engine e1(r());
	for (int i = 0; i < 10; i++)
	{
		auto cur_array_rank = array_rank("temp", rank_num, pool_num, -100000, 100000);
		auto cur_list_rank = skiplist_rank("temp", rank_num, pool_num, -100000, 100000);
		std::shuffle(temp_ranks.begin(), temp_ranks.end(), e1);
		for (const auto& one_info : temp_ranks)
		{
			cur_array_rank.update(one_info);
			cur_list_rank.update(one_info);
		}
		auto top_list1 = cur_array_rank.get_players(1, rank_num);
		auto top_list2 = cur_list_rank.get_players(1, rank_num);
		for (int j = 0; j < rank_num; j++)
		{
			auto cur_player_id = "player_" + std::to_string(j);
			auto cur_player_rank = cur_array_rank.get_rank(cur_player_id);
			auto cur_player_rank2 = cur_list_rank.get_rank(cur_player_id);
			assert(cur_player_rank.second  == (j + 1) && cur_player_rank.second == cur_player_rank2.second);

			assert(cur_array_rank.get_rank(-1.0 * j) == cur_list_rank.get_rank(-1.0 * j));
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
		auto cur_array_rank = array_rank("temp", rank_num, pool_num, -100000, 100000);
		auto cur_list_rank = skiplist_rank("temp", rank_num, pool_num, -100000, 100000);
		// std::shuffle(temp_ranks.begin(), temp_ranks.end(), e1);
		for (const auto& one_info : temp_ranks)
		{
			cur_array_rank.update(one_info);
			cur_list_rank.update(one_info);
		}
		for (int j = 0; j < rank_num; j++)
		{
			auto cur_player_id = "player_" + std::to_string(j);
			auto cur_player_rank = cur_array_rank.get_rank(cur_player_id);
			auto cur_player_rank2 = cur_list_rank.get_rank(cur_player_id);
			assert(cur_player_rank.second == (j + 1) && cur_player_rank.second == cur_player_rank2.second);
			auto temp_rank_1 = cur_array_rank.get_rank(-1.0 * i);
			auto temp_rank_2 = cur_list_rank.get_rank(-1.0 * i);
			if (temp_rank_1 > 0 && temp_rank_1 <= cur_array_rank.m_rank_sz)
			{
				assert(temp_rank_1 == temp_rank_2);
			}
			else
			{
				assert(temp_rank_2 > cur_array_rank.m_rank_sz);
			}
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
		cur_rank_info.update_ts = i;
		temp_ranks.push_back(cur_rank_info);
	}
	
	auto cur_array_rank = array_rank("temp", rank_num, pool_num, -100000, 100000);
	auto cur_list_rank = skiplist_rank("temp", rank_num, pool_num, -100000, 100000);
	for (auto one_data : temp_ranks)
	{
		cur_array_rank.update(one_data);
		cur_list_rank.update(one_data);
		assert(cur_array_rank.size() == cur_list_rank.size());
	}
	std::sort(temp_ranks.begin(), temp_ranks.end());
	for (int i = 0; i < temp_ranks.size(); i++)
	{
		auto cur_player_rank = cur_array_rank.get_rank(temp_ranks[i].player_id);
		auto cur_player_rank2 = cur_list_rank.get_rank(temp_ranks[i].player_id);
		if (cur_player_rank.second > 0 && cur_player_rank.second <= cur_array_rank.m_rank_sz)
		{
			assert(cur_player_rank.second == (i + 1) && cur_player_rank.second == cur_player_rank2.second);
		}
		
	}
	cur_list_rank.debug_print();
	std::shuffle(temp_ranks.begin(), temp_ranks.end(), e1);
	for (int i = 0; i < pool_num; i++)
	{
		auto cur_back = temp_ranks.back();
		cur_array_rank.remove(cur_back.player_id);
		cur_list_rank.remove(cur_back.player_id);
		// assert(cur_array_rank.size() == cur_list_rank.size());
		temp_ranks.pop_back();
	}
	cur_list_rank.debug_print();
	std::sort(temp_ranks.begin(), temp_ranks.end());
	for (int i = 0; i < temp_ranks.size(); i++)
	{
		auto cur_player_rank = cur_array_rank.get_rank(temp_ranks[i].player_id);
		auto cur_player_rank2 = cur_list_rank.get_rank(temp_ranks[i].player_id);
		if (cur_player_rank.second > 0 && cur_player_rank.second <= cur_array_rank.m_rank_sz)
		{
			assert(cur_player_rank.second == (i + 1) && cur_player_rank.second == cur_player_rank2.second);
		}
		auto temp_rank_1 = cur_array_rank.get_rank(-1.0 * i);
		auto temp_rank_2 = cur_list_rank.get_rank(-1.0 * i);
		if (temp_rank_1 > 0 && temp_rank_1 <= cur_array_rank.m_rank_sz)
		{
			assert(temp_rank_1 == temp_rank_2);
		}
		else
		{
			assert(temp_rank_2 > cur_array_rank.m_rank_sz);
		}
		
	}
}

void test_4()
{
	std::uint32_t rank_num = 100;
	std::uint32_t pool_num = 200;
	std::random_device r;
	std::default_random_engine e1(r());
	std::uniform_int_distribution<int> int_dis(1, 100);

	std::uniform_int_distribution<int> delta_dis(-50, 50);

	std::vector<rank_info> temp_ranks;
	for (int i = 0; i < 2 * pool_num; i++)
	{
		rank_info cur_rank_info;
		cur_rank_info.player_id = "player_" + std::to_string(i);
		cur_rank_info.rank_value = int_dis(e1);
		cur_rank_info.update_ts = i;
		temp_ranks.push_back(cur_rank_info);
	}

	auto cur_array_rank = array_rank("temp", rank_num, pool_num, -100000, 100000);
	auto cur_list_rank = skiplist_rank("temp", rank_num, pool_num, -100000, 100000);
	for (auto one_data : temp_ranks)
	{
		cur_array_rank.update(one_data);
		cur_list_rank.update(one_data);
		assert(cur_array_rank.size() == cur_list_rank.size());
	}
	std::sort(temp_ranks.begin(), temp_ranks.end());
	for (int i = 0; i < temp_ranks.size(); i++)
	{
		auto cur_player_rank = cur_array_rank.get_rank(temp_ranks[i].player_id);
		auto cur_player_rank2 = cur_list_rank.get_rank(temp_ranks[i].player_id);
		if (cur_player_rank.second > 0 && cur_player_rank.second <= cur_array_rank.m_rank_sz)
		{
			assert(cur_player_rank.second == (i + 1) && cur_player_rank.second == cur_player_rank2.second);
		}

	}
	cur_list_rank.debug_print();
	std::shuffle(temp_ranks.begin(), temp_ranks.end(), e1);
	for (auto& one_data : temp_ranks)
	{
		one_data.rank_value += delta_dis(e1);
		auto temp_rank_1 = cur_array_rank.update(one_data);
		auto temp_rank_2 = cur_list_rank.update(one_data);
		if (temp_rank_1 > 0 && temp_rank_1 <= cur_array_rank.m_rank_sz)
		{
			assert(temp_rank_1 == temp_rank_2);
		}
	}
	for (int i = 0; i < pool_num; i++)
	{
		auto cur_back = temp_ranks.back();
		cur_array_rank.remove(cur_back.player_id);
		cur_list_rank.remove(cur_back.player_id);
		// assert(cur_array_rank.size() == cur_list_rank.size());
		temp_ranks.pop_back();
	}
	cur_list_rank.debug_print();
	std::sort(temp_ranks.begin(), temp_ranks.end());
	for (int i = 0; i < temp_ranks.size(); i++)
	{
		auto cur_player_rank = cur_array_rank.get_rank(temp_ranks[i].player_id);
		auto cur_player_rank2 = cur_list_rank.get_rank(temp_ranks[i].player_id);
		if (cur_player_rank.second > 0 && cur_player_rank.second <= cur_array_rank.m_rank_sz)
		{
			assert(cur_player_rank.second == cur_player_rank2.second);
		}
		auto temp_rank_1 = cur_array_rank.get_rank(-1.0 * i);
		auto temp_rank_2 = cur_list_rank.get_rank(-1.0 * i);
		if (temp_rank_1 > 0 && temp_rank_1 <= cur_array_rank.m_rank_sz)
		{
			assert(temp_rank_1 == temp_rank_2);
		}
		else
		{
			assert(temp_rank_2 > cur_array_rank.m_rank_sz);
		}

	}
}

int main()
{
	test_1();
	test_2();
	test_3();
	test_4();
	return 1;
}