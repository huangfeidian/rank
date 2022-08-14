#include "rank_list.h"


namespace spiritsaway::system::rank
{
	rank_list::rank_list(const std::string& name, const std::uint32_t rank_sz, const std::uint32_t pool_sz)
		: m_name(name)
		, m_rank_sz(rank_sz)
		, m_pool_sz(pool_sz)
		, m_rank_info_pool(pool_sz)
		, m_pool_unused_indexes(pool_sz)
	{
		for (std::uint32_t i = 0; i < pool_sz; i++)
		{
			m_pool_unused_indexes[i] = i;
		}
		std::reverse(m_pool_unused_indexes.begin(), m_pool_unused_indexes.end());
		m_player_to_pool_idx.reserve(2 * m_pool_sz);
		m_sorted_rank_ptrs.reserve(m_pool_sz);
	}
	bool rank_list::remove(const std::string& player_id)
	{
		auto temp_iter = m_player_to_pool_idx.find(player_id);
		if (temp_iter == m_player_to_pool_idx.end())
		{
			return false;
		}
		const auto& one_player = m_rank_info_pool[temp_iter->second];


		auto vec_iter = std::lower_bound(m_sorted_rank_ptrs.begin(), m_sorted_rank_ptrs.end(), rank_info_ptr_wrapper{ &one_player });
		m_sorted_rank_ptrs.erase(vec_iter);
		m_rank_info_pool[temp_iter->second].clear();
		m_pool_unused_indexes.push_back(temp_iter->second);
		m_player_to_pool_idx.erase(temp_iter);
		return true;
	}
	std::optional<std::uint32_t> rank_list::rank(const std::string& player_id)
	{
		auto temp_iter = m_player_to_pool_idx.find(player_id);
		if (temp_iter == m_player_to_pool_idx.end())
		{
			return {};
		}
		const auto& one_player = m_rank_info_pool[temp_iter->second];


		auto vec_iter = std::lower_bound(m_sorted_rank_ptrs.begin(), m_sorted_rank_ptrs.end(), rank_info_ptr_wrapper{ &one_player });
		auto cur_rank = std::distance(m_sorted_rank_ptrs.begin(), vec_iter);
		if (cur_rank >= m_rank_sz)
		{
			return {};
		}
		else
		{
			return cur_rank;
		}

	}
	void rank_list::reset(const std::vector<rank_info>& player_ranks)
	{
		m_player_to_pool_idx.clear();
		m_sorted_rank_ptrs.clear();
		for (auto& one_rank : m_rank_info_pool)
		{
			one_rank.clear();
		}
		m_pool_unused_indexes.clear();
		for (std::uint32_t i = 0; i < m_pool_sz; i++)
		{
			m_pool_unused_indexes.push_back(i);
		}
		std::reverse(m_pool_unused_indexes.begin(), m_pool_unused_indexes.end());
		for (const auto& one_player : player_ranks)
		{
			if (m_pool_unused_indexes.empty())
			{
				return;
			}
			auto cur_idx = m_pool_unused_indexes.back();
			m_pool_unused_indexes.pop_back();
			m_rank_info_pool[cur_idx] = one_player;
			m_sorted_rank_ptrs.push_back(rank_info_ptr_wrapper{ &m_rank_info_pool[cur_idx] });
			m_player_to_pool_idx[one_player.player_id] = cur_idx;
			
		}
	}

	rank_list::update_result rank_list::update(const rank_info& one_player)
	{
		auto temp_iter = m_player_to_pool_idx.find(one_player.player_id);
		
		if (temp_iter == m_player_to_pool_idx.end())
		{
			if (m_sorted_rank_ptrs.size() == m_pool_sz)
			{
				if (*m_sorted_rank_ptrs.back().ptr < one_player)
				{
					return update_result::not_in_rank;
				}
				else
				{
					auto pre_idx_iter = m_player_to_pool_idx.find(m_sorted_rank_ptrs.back().ptr->player_id);
					m_pool_unused_indexes.push_back(pre_idx_iter->second);
					m_rank_info_pool[pre_idx_iter->second].clear();
					m_player_to_pool_idx.erase(pre_idx_iter);
					m_sorted_rank_ptrs.pop_back();
				}
			}
			auto cur_player_idx = m_pool_unused_indexes.back();
			m_pool_unused_indexes.pop_back();
			m_player_to_pool_idx[one_player.player_id] = cur_player_idx;
			m_rank_info_pool[cur_player_idx] = one_player;
			auto cur_key = rank_info_ptr_wrapper{ &m_rank_info_pool[cur_player_idx] };

			auto cur_insert_iter = std::lower_bound(m_sorted_rank_ptrs.begin(), m_sorted_rank_ptrs.end(), cur_key);
			m_sorted_rank_ptrs.insert(cur_insert_iter, cur_key);
			return update_result::add_to_rank;
		}
		auto cur_key = rank_info_ptr_wrapper{ &one_player };
		auto pre_key = rank_info_ptr_wrapper{ &m_rank_info_pool[temp_iter->second] };
		bool is_key_increase = pre_key < cur_key;
		auto sorted_iter = std::lower_bound(m_sorted_rank_ptrs.begin(), m_sorted_rank_ptrs.end(), pre_key);
		m_rank_info_pool[temp_iter->second] = one_player;
		if (is_key_increase)
		{
			auto next_iter = sorted_iter + 1;
			while (next_iter != m_sorted_rank_ptrs.end() && (*next_iter < pre_key))
			{
				std::swap(*next_iter, *(next_iter - 1));
				next_iter = next_iter + 1;
			}
		}
		else
		{
			auto pre_iter = sorted_iter;
			while (pre_iter != m_sorted_rank_ptrs.begin())
			{
				pre_iter = pre_iter - 1;
				if (*pre_iter < pre_key)
				{
					break;
				}
				std::swap(*pre_iter, *(pre_iter + 1));

			}
		}
		return update_result::remain_in_rank;
	}

	json rank_list::encode() const
	{
		std::vector< rank_info> temp_sorted_rank_info;
		temp_sorted_rank_info.reserve(m_sorted_rank_ptrs.size());
		for (const auto& one_key : m_sorted_rank_ptrs)
		{
			temp_sorted_rank_info.push_back(*one_key.ptr);
			
		}
		json result;
		result["name"] = m_name;
		result["rank_sz"] = m_rank_sz;
		result["pool_sz"] = m_pool_sz;
		result["sorted_ranks"] = temp_sorted_rank_info;
		return result;
	}

	std::unique_ptr<rank_list> rank_list::create(const json& data)
	{
		std::uint32_t rank_sz;
		std::uint32_t pool_sz;
		std::string name;
		std::vector< rank_info> temp_sorted_rank_info;
		try
		{
			data.at("name").get_to(name);
			data.at("pool_sz").get_to(pool_sz);
			data.at("rank_sz").get_to(rank_sz);
			data.at("sorted_ranks").get_to(temp_sorted_rank_info);
		}
		catch (std::exception& e)
		{
			assert(false);
			return {};
		}
		auto cur_list = std::make_unique<rank_list>(name, rank_sz, pool_sz);
		cur_list->reset(temp_sorted_rank_info);
		return std::move(cur_list);
	}

}