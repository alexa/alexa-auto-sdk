package com.amazon.alexa.auto.navigation.poi;

import android.annotation.SuppressLint;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.amazon.alexa.auto.aacs.common.PointOfInterest;
import com.amazon.alexa.auto.navigation.R;
import com.amazon.alexa.auto.navigation.handlers.LocalSearchDirectiveHandler;
import com.amazon.alexa.auto.navigation.providers.NavigationProvider;
import com.bumptech.glide.Glide;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

/**
 * RecyclerView.Adapter for rendering the POI List Template Runtime template.
 */
public class LocalSearchListAdapter extends RecyclerView.Adapter<LocalSearchListAdapter.ViewHolder> {
    private static final String TAG = LocalSearchListAdapter.class.getSimpleName();
    private final NavigationProvider mNavigationProvider;
    private final List<PointOfInterest> mPOIs = new ArrayList<>();
    private final WeakReference<LocalSearchDirectiveHandler> mDirectiveHandlerWeakReference;
    private final Context mContext;

    public LocalSearchListAdapter(NavigationProvider navigationProvider,
            WeakReference<LocalSearchDirectiveHandler> directiveHandlerWeakReference, Context context) {
        mNavigationProvider = navigationProvider;
        mDirectiveHandlerWeakReference = directiveHandlerWeakReference;
        mContext = context;
    }

    /**
     * Add a POI to the list.
     * @param poi point of interest.
     */
    public void addPOI(PointOfInterest poi) {
        mPOIs.add(poi);
    }

    /**
     * Gets all the POIs in the list.
     * @return list of POIs.
     */
    public List<PointOfInterest> getPOIs() {
        return mPOIs;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        LayoutInflater inflater = LayoutInflater.from(parent.getContext());
        View poiItemView = inflater.inflate(R.layout.local_search_item, parent, false);
        return new ViewHolder(poiItemView);
    }

    @SuppressLint("SetTextI18n")
    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        final PointOfInterest poi = mPOIs.get(position);
        holder.mPOIOrdinal.setText(Integer.toString(position + 1));
        holder.mPOIName.setText(poi.getTitle().getMainTitle());
        holder.mPOIAddress.setText(poi.getAddress());
        holder.mEta.setText(poi.getTravelTime());
        holder.mPoiRoot.setOnClickListener(poiNameView -> {
            mNavigationProvider.startNavigation(poi);
            if (mDirectiveHandlerWeakReference.get() != null) {
                mDirectiveHandlerWeakReference.get().clearTemplate();
            }
        });

        if (poi.getProvider().equals("Yelp") && poi.getRating().getImage().getSources().size() >= 1) {
            String yelpImageUrl = poi.getRating().getImage().getSources().get(0).getUrl();
            Glide.with(mContext).load(yelpImageUrl).into(holder.mYelpImage);
            holder.mYelpRatingCount.setText(String.format("(%s)", poi.getRating().getReviewCount()));
        } else {
            holder.mYelpRow.setVisibility(View.GONE);
        }

        if (position == (mPOIs.size() - 1)) {
            holder.mSplitter.setVisibility(View.GONE);
        }
    }

    @Override
    public int getItemCount() {
        return mPOIs.size();
    }

    static class ViewHolder extends RecyclerView.ViewHolder {
        public TextView mYelpRatingCount;
        public ImageView mYelpImage;
        public View mYelpRow;
        public TextView mPOIOrdinal;
        public TextView mPOIAddress;
        public TextView mPOIName;
        public TextView mEta;
        public View mSplitter;
        public View mPoiRoot;
        public ViewHolder(@NonNull View itemView) {
            super(itemView);
            mPOIOrdinal = itemView.findViewById(R.id.poi_ordinal);
            mPOIName = itemView.findViewById(R.id.poi_name);
            mPOIAddress = itemView.findViewById(R.id.poi_address);
            mEta = itemView.findViewById(R.id.poi_list_eta);
            mSplitter = itemView.findViewById(R.id.poi_splitter);
            mPoiRoot = itemView.findViewById(R.id.poi_item_root);
            mYelpRow = itemView.findViewById(R.id.yelp_rating_row);
            mYelpImage = itemView.findViewById(R.id.yelp_rating_image);
            mYelpRatingCount = itemView.findViewById(R.id.yelp_rating_count);
        }
    }
}
