#include "rank_list.h"
#include <random>

using namespace spiritsaway::system::rank;

void test_1()
{
	std::uint32_t rank_num = 100;
	std::uint32_t pool_num = 200;
	std::vector<rank_info> original_ranks;
	for (int i = 0; i < 2 * pool_num; i++)
	{
		rank_info cur_rank_info;
		cur_rank_info.player_id = "player_" + std::to_string(i);
		cur_rank_info.rank_value = -1.0 * i;
		cur_rank_info.update_ts = 2 * pool_num - i;
		original_ranks.push_back(cur_rank_info);
	}
	std::random_device r;
	std::default_random_engine e1(r());
	std::uniform_real_distribution<double> f_dis(1.0, 50.0);
	for (int i = 0; i < 10; i++)
	{
		auto temp_ranks = original_ranks;
		auto cur_rank_list = rank_list("temp", rank_num, pool_num);
		auto sync_rank_list = rank_list("temp", rank_num, pool_num);
		std::shuffle(temp_ranks.begin(), temp_ranks.end(), e1);
		for (const auto& one_info : temp_ranks)
		{
			auto cur_update_result = cur_rank_list.update(one_info);
			if (cur_update_result == rank_list::update_result::not_in_rank)
			{
				continue;
			}
			sync_rank_list.update(one_info);
		}

		for (int j = 0; j < rank_num; j++)
		{
			auto cur_player_id = "player_" + std::to_string(j);
			auto cur_player_rank = cur_rank_list.rank(cur_player_id);
			auto sync_player_rank = sync_rank_list.rank(cur_player_id);
			assert(cur_player_rank && cur_player_rank.value() == j && cur_player_rank == sync_player_rank);
		}
		for (const auto& one_info : temp_ranks)
		{
			if (cur_rank_list.remove(one_info.player_id))
			{
				sync_rank_list.remove(one_info.player_id);
				for (int j = 0; j < rank_num; j++)
				{
					auto cur_player_id = "player_" + std::to_string(j);
					auto cur_player_rank = cur_rank_list.rank(cur_player_id);
					auto sync_player_rank = sync_rank_list.rank(cur_player_id);
					assert(cur_player_rank == sync_player_rank);
				}
			}
		}
	}
}

void test_2()
{

	std::uint32_t rank_num = 100;
	std::uint32_t pool_num = 200;
	std::vector<rank_info> original_ranks;
	for (int i = 0; i < 2 * pool_num; i++)
	{
		rank_info cur_rank_info;
		cur_rank_info.player_id = "player_" + std::to_string(i);
		cur_rank_info.rank_value = -1.0 * i;
		cur_rank_info.update_ts = 2 * pool_num - i;
		original_ranks.push_back(cur_rank_info);
	}
	std::random_device r;
	std::default_random_engine e1(r());
	std::uniform_real_distribution<double> f_dis(1.0, 50.0);
	for (int i = 0; i < 10; i++)
	{
		auto temp_ranks = original_ranks;
		auto cur_rank_list = rank_list("temp", rank_num, pool_num);
		auto sync_rank_list = rank_list("temp", rank_num, pool_num);
		std::shuffle(temp_ranks.begin(), temp_ranks.end(), e1);
		for (const auto& one_info : temp_ranks)
		{
			auto cur_update_result = cur_rank_list.update(one_info);
			if (cur_update_result == rank_list::update_result::not_in_rank)
			{
				continue;
			}
			sync_rank_list.update(one_info);
		}

		for (int j = 0; j < rank_num; j++)
		{
			auto cur_player_id = "player_" + std::to_string(j);
			auto cur_player_rank = cur_rank_list.rank(cur_player_id);
			auto sync_player_rank = sync_rank_list.rank(cur_player_id);
			assert(cur_player_rank && cur_player_rank.value() == j && cur_player_rank == sync_player_rank);
		}
		for (auto& one_info : temp_ranks)
		{
			one_info.rank_value += f_dis(e1);
			if (cur_rank_list.update(one_info) != rank_list::update_result::not_in_rank)
			{
				sync_rank_list.update(one_info);
				for (int j = 0; j < rank_num; j++)
				{
					auto cur_player_id = "player_" + std::to_string(j);
					auto cur_player_rank = cur_rank_list.rank(cur_player_id);
					auto sync_player_rank = sync_rank_list.rank(cur_player_id);
					assert(cur_player_rank == sync_player_rank);
				}
			}
		}
	}
}

int main()
{
	test_1();
	test_2();
	return 0;
}