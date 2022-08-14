#include "rank_list.h"


namespace spiritsaway::system::rank
{
	rank_list::rank_list(const std::string& name, const std::uint32_t rank_sz, const std::uint32_t pool_sz)
		: m_name(name)
		, m_rank_sz(rank_sz)
		, m_pool_sz(pool_sz)
	{
		m_player_rank_infos.reserve(2 * m_pool_sz);
		m_sorted_player_keys.reserve(m_pool_sz);
	}
	bool rank_list::remove(const std::string& player_id)
	{
		auto temp_iter = m_player_rank_infos.find(player_id);
		if (temp_iter == m_player_rank_infos.end())
		{
			return false;
		}
		const auto& one_player = temp_iter->second;


		auto vec_iter = std::lower_bound(m_sorted_player_keys.begin(), m_sorted_player_keys.end(), rank_info_ptr_wrapper{one_player.get()});
		m_sorted_player_keys.erase(vec_iter);
		m_player_rank_infos.erase(temp_iter);
		return true;
	}
	std::optional<std::uint32_t> rank_list::rank(const std::string& player_id)
	{
		auto temp_iter = m_player_rank_infos.find(player_id);
		if (temp_iter == m_player_rank_infos.end())
		{
			return {};
		}
		const auto& one_player = temp_iter->second;


		auto vec_iter = std::lower_bound(m_sorted_player_keys.begin(), m_sorted_player_keys.end(), rank_info_ptr_wrapper{ one_player.get() });
		auto cur_rank = std::distance(m_sorted_player_keys.begin(), vec_iter);
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
		m_player_rank_infos.clear();
		m_sorted_player_keys.clear();

		for (const auto& one_player : player_ranks)
		{
			auto one_player_ptr = std::make_unique<rank_info>(one_player);
			m_sorted_player_keys.push_back(rank_info_ptr_wrapper{ one_player_ptr.get() });
			m_player_rank_infos[one_player.player_id] = std::move(one_player_ptr);
			
		}
	}

	rank_list::update_result rank_list::update(const rank_info& one_player)
	{
		auto temp_iter = m_player_rank_infos.find(one_player.player_id);
		auto cur_key = rank_info_ptr_wrapper{&one_player};
		if (temp_iter == m_player_rank_infos.end())
		{
			if (m_sorted_player_keys.size() == m_pool_sz)
			{
				if (m_sorted_player_keys.back() < cur_key)
				{
					return update_result::not_in_rank;
				}
				else
				{
					m_player_rank_infos.erase(m_sorted_player_keys.back().ptr->player_id);
					m_sorted_player_keys.pop_back();
				}
			}
			auto one_player_ptr = std::make_unique<rank_info>(one_player);
			cur_key = rank_info_ptr_wrapper{ one_player_ptr.get() };
			m_player_rank_infos[one_player.player_id] = std::move(one_player_ptr);
			auto cur_insert_iter = std::lower_bound(m_sorted_player_keys.begin(), m_sorted_player_keys.end(), cur_key);
			m_sorted_player_keys.insert(cur_insert_iter, cur_key);
			return update_result::add_to_rank;
		}
		auto pre_key = rank_info_ptr_wrapper{ temp_iter->second.get()};
		bool is_key_increase = pre_key < cur_key;
		auto sorted_iter = std::lower_bound(m_sorted_player_keys.begin(), m_sorted_player_keys.end(), pre_key);
		*(temp_iter->second) = one_player;
		if (is_key_increase)
		{
			auto next_iter = sorted_iter + 1;
			while (next_iter != m_sorted_player_keys.end() && (*next_iter < pre_key))
			{
				std::swap(*next_iter, *(next_iter - 1));
				next_iter = next_iter + 1;
			}
		}
		else
		{
			auto pre_iter = sorted_iter;
			while (pre_iter != m_sorted_player_keys.begin())
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
		temp_sorted_rank_info.reserve(m_sorted_player_keys.size());
		for (const auto& one_key : m_sorted_player_keys)
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