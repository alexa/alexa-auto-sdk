/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

package com.amazon.sampleapp.logView;

import android.graphics.Color;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.amazon.sampleapp.R;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.HashMap;

class ConfigureViewHolder {
    private static String sTag = "CLI";
    private static int sNumForecastItems = 5; // For weather template card
    private static int sNumListItems = 5; // For list template card
    private static int sNumLocalSearchListItems = 4; // For local search template card

    static void configureTextLog( ViewHolderTextLog vh, JSONObject json ) {
        try {
            String text = ( String ) json.get( "text" );
            int color = ( int ) json.get( "textColor" );
            vh.getLog().setText( text );
            vh.setHighlight( color );
        } catch ( JSONException e ) { Log.e( sTag, e.getMessage() ); }
    }

    static void configureBodyTemplate1( ViewHolderBodyTemplate1 vh, JSONObject json ) {
        try {
            if( json.has("template") ) {
                JSONObject template = json.getJSONObject( "template" );

                if ( template.has( "title" ) ) {
                    JSONObject title = template.getJSONObject( "title" );
                    if ( title.has( "mainTitle" ) ) {
                        String mainTitle = title.getString( "mainTitle" );
                        vh.getMainTitle().setText( mainTitle );
                    }

                    if ( title.has( "subTitle" ) ) {
                        String subTitle = title.getString( "subTitle" );
                        vh.getSubTitle().setText( subTitle );
                    }
                }

                if ( template.has( "textField" ) ) {
                    String textField = template.getString( "textField" );
                    vh.getTextField().setText( textField );
                }
            }
        } catch ( JSONException e ) { Log.e( sTag, e.getMessage() ); }
    }

    static void configureBodyTemplate2( ViewHolderBodyTemplate2 vh, JSONObject json ) {
        try {
            if( json.has( "template" ) ) {
                JSONObject template = json.getJSONObject( "template" );

                if ( template.has( "title" ) ) {
                    JSONObject title = template.getJSONObject( "title" );
                    if ( title.has( "mainTitle" ) ) {
                        String mainTitle = title.getString( "mainTitle" );
                        vh.getMainTitle().setText( mainTitle );
                    }

                    if ( title.has( "subTitle" ) ) {
                        String subTitle = title.getString( "subTitle" );
                        vh.getSubTitle().setText( subTitle );
                    }
                }

                if ( template.has( "textField" ) ) {
                    String textField = template.getString( "textField" );
                    vh.getTextField().setText( textField );
                }

                if ( template.has( "image" ) ) {
                    JSONObject image = template.getJSONObject( "image" );
                    String imageURL = getImageUrl( image );
                    new DownloadImageTask( vh.getImage() ).execute( imageURL );
                }
            }
        } catch ( JSONException e ) { Log.e( sTag, e.getMessage() ); }
    }

    static void configureListTemplate1( ViewHolderListTemplate1 vh, JSONObject json ) {
        try {

            if ( json.has( "template" ) ) {
                JSONObject template = json.getJSONObject( "template" );

                if ( template.has( "title" ) ) {
                    JSONObject title = template.getJSONObject( "title" );
                    if ( title.has( "mainTitle" ) ) {
                        String mainTitle = title.getString( "mainTitle" );
                        vh.getMainTitle().setText( mainTitle );
                    }

                    if ( title.has( "subTitle" ) ) {
                        String subTitle = title.getString( "subTitle" );
                        vh.getSubTitle().setText( subTitle );
                    }
                }

                if ( template.has( "listItems" ) ) {
                    JSONArray listItems = template.getJSONArray( "listItems" );

                    // Truncate list
                    int numItems = listItems.length() > sNumListItems
                            ? sNumListItems : listItems.length();

                    vh.clearLists();
                    for ( int j = 0; j < numItems; j++ ) {
                        JSONObject nextItem = listItems.getJSONObject( j );
                        String index = nextItem.has( "leftTextField" )
                                ? nextItem.getString( "leftTextField" ) : "";
                        String content = nextItem.has( "rightTextField" )
                                ? nextItem.getString( "rightTextField" ) : "";
                        vh.insertListItem( index, content );
                    }
                }
            }
        } catch ( JSONException e ) { Log.e( sTag, e.getMessage() ); }
    }

    static void configureWeatherTemplate( ViewHolderWeatherTemplate vh, JSONObject json ) {
        try {
            if ( json.has( "template" ) ) {
                JSONObject template = json.getJSONObject( "template" );

                if ( template.has( "title" ) ) {
                    JSONObject title = template.getJSONObject( "title" );
                    if ( title.has( "mainTitle" ) ) {
                        String mainTitle = title.getString( "mainTitle" );
                        vh.getMainTitle().setText( mainTitle );
                    }

                    if ( title.has( "subTitle" ) ) {
                        String subTitle = title.getString( "subTitle" );
                        vh.getSubTitle().setText( subTitle );
                    }
                }

                if ( template.has( "currentWeather" ) ) {
                    String currentWeather = template.getString( "currentWeather" );
                    vh.getCurrentWeather().setText( currentWeather );
                }

                if ( template.has( "currentWeatherIcon" ) ) {
                    String currentWeatherIconURL =
                            getImageUrl( template.getJSONObject( "currentWeatherIcon" ) );
                    new DownloadImageTask( vh.getCurrentWeatherIcon() ).execute( currentWeatherIconURL );
                }

                if ( template.has( "highTemperature" ) ) {
                    String highTempValue =
                            template.getJSONObject( "highTemperature" ).getString( "value" );
                    vh.getHighTemp().setText( highTempValue );
                }

                if ( template.has( "lowTemperature" ) ) {
                    String lowTempValue =
                            template.getJSONObject( "lowTemperature" ).getString( "value" );
                    vh.getLowTemp().setText( lowTempValue );
                }

                if ( template.has( "weatherForecast" ) ) {
                    JSONArray forecasts = template.getJSONArray( "weatherForecast" );
                    for ( int j = 0; j < sNumForecastItems; j++ ) {
                        // Get forecast
                        JSONObject next = forecasts.getJSONObject( j );
                        View forecastView = vh.getForecast( j );

                        // Set icon
                        JSONObject image = next.getJSONObject( "image" );
                        String url = getImageUrl( image );
                        new DownloadImageTask(
                                ( ImageView ) forecastView.findViewById( R.id.forecastIcon )
                        ).execute( url );

                        // Set day
                        String day = next.has( "day" ) ? next.getString( "day" ) : "";
                        ( ( TextView ) forecastView.findViewById( R.id.day ) ).setText( day );

                        // Set high temp
                        String high = next.has("highTemperature")
                                ? next.getString("highTemperature") : "";
                        ( ( TextView ) forecastView.findViewById( R.id.highTemp ) ).setText( high );

                        // Set low temp
                        String low = next.has("lowTemperature")
                                ? next.getString("lowTemperature") : "";
                        ( ( TextView ) forecastView.findViewById( R.id.lowTemp ) ).setText( low );
                    }
                }
            }
        } catch ( JSONException e ) { Log.e( sTag, e.getMessage() ); }
    }

    static void configureSetDestinationTemplate( ViewHolderSetDestinationTemplate vh, JSONObject json ) {
        try {
            if ( json.has( "template" ) ) {
                JSONObject template = json.getJSONObject( "template" );

                if ( template.has( "destination" ) ) {
                    JSONObject dest = template.getJSONObject( "destination" );

                    String name = dest.has( "name" ) ? dest.getString( "name" ) : "";
                    vh.getName().setText( name );

                    String address = dest.has( "singleLineDisplayAddress" )
                            ? dest.getString( "singleLineDisplayAddress" ) : "";
                    vh.getAddress().setText( address );

                    if ( dest.has( "coordinate" ) ) {
                        JSONObject coord = dest.getJSONObject( "coordinate" );
                        double lat = Double.parseDouble( coord.getString( "latitudeInDegrees" ) );
                        double lng = Double.parseDouble( coord.getString( "longitudeInDegrees" ) );
                        vh.setPlace( lat,lng );
                    }
                }
            }
        } catch ( JSONException e ) { Log.e( sTag, e.getMessage() ); }
    }

    static void configureLocalSearchListTemplate1( ViewHolderLocalSearchListTemplate1 vh,
                                                   JSONObject json ) {
        try {
            if ( json.has( "template" ) ) {
                JSONObject template = json.getJSONObject("template");

                if ( template.has( "listItems" ) ) {
                    JSONArray listItems = template.getJSONArray( "listItems" );

                    // Truncate list
                    int numListItems = listItems.length() > sNumLocalSearchListItems
                            ? sNumLocalSearchListItems : listItems.length();

                    vh.clearList();
                    for ( int j = 0; j < numListItems; j++ ) {
                        JSONObject listItem = listItems.getJSONObject( j );

                        String dist = listItem.has( "leftTextField" )
                                ? listItem.getString( "leftTextField" ) : "";
                        String name = listItem.has( "rightPrimaryTextField" )
                                ? listItem.getString( "rightPrimaryTextField" ) : "";
                        String address = listItem.has( "rightSecondaryTextField" )
                                ? listItem.getString( "rightSecondaryTextField" ) : "";
                        String index = "" + ( j + 1 );
                        vh.insertListItem( index, dist, name, address );
                    }
                }
            }
        } catch ( JSONException e ) { Log.e( sTag, e.getMessage() ); }
    }

    static void configureRenderPlayerInfo( ViewHolderRenderPlayerInfo vh, JSONObject json ) {
        try {
            if ( json.has( "template" ) ) {
                JSONObject template = json.getJSONObject( "template" );

                String header = template.has( "header" )
                        ? template.getString( "header" ) : "";
                vh.getHeader().setText( header );

                String headerSubtext1 = "";
                if ( template.has( "headerSubtext1" ) ) {
                    headerSubtext1 = template.getString( "headerSubtext1" );
                } else if ( template.has( "provider" ) ) {
                    // Set header subtext to provider name if no header subtext given
                    JSONObject provider = template.getJSONObject( "provider" );
                    if ( provider.has( "name" ) ) {
                        headerSubtext1 = provider.getString( "name" );
                    }
                }
                vh.getHeaderSubtext1().setText( headerSubtext1 );

                String title = template.has( "title" )
                        ? template.getString( "title" ) : "";
                vh.getTitle().setText( title );

                String titleSubtext1 = template.has( "titleSubtext1" )
                        ? template.getString( "titleSubtext1" ) : "";
                vh.getTitleSubtext1().setText( titleSubtext1 );

                String titleSubtext2 = template.has( "titleSubtext2" )
                        ? template.getString( "titleSubtext2" ) : "";
                vh.getTitleSubtext2().setText( titleSubtext2 );

                if ( template.has( "art" ) ) {
                    JSONObject art = template.getJSONObject( "art" );
                    String url = getImageUrl( art );
                    new DownloadImageTask( vh.getArt() ).execute( url );
                } else {
                    vh.getArt().setImageDrawable( null );
                }

                if ( template.has( "provider" ) ) {
                    JSONObject provider = template.getJSONObject( "provider" );
                    if ( provider.has( "logo" ) ) {
                        JSONObject logo = provider.getJSONObject( "logo" );
                        String url = getImageUrl( logo );
                        new DownloadImageTask( vh.getPartnerLogo() ).execute( url );
                    } else {
                        vh.getPartnerLogo().setImageDrawable( null );
                    }
                }
            }
        } catch ( JSONException e ) { Log.e( sTag, e.getMessage() ); }

    }

    static void configureCBLCard( ViewHolderCBLCard vh, JSONObject json ) {
        try {
            JSONObject template = json.getJSONObject( "template" );
            String userCode = template.has( "user_code" )
                    ? template.getString( "user_code" ) : "";
            String url = template.has( "verification_uri" )
                    ? template.getString( "verification_uri" ) : "";
            vh.setMessage( url );
            vh.setCode( userCode );
        } catch ( JSONException e ) { Log.e( sTag, e.getMessage() ); }

    }

    static void configureCBLExpiredCard( ViewHolderCBLExpiredCard vh, JSONObject json ) {
        try {
            JSONObject template = json.getJSONObject( "template" );
            String message = template.has( "message" )
                    ? template.getString( "message" ) : "";
            vh.setMessage( message );
        } catch ( JSONException e ) { Log.e( sTag, e.getMessage() ); }
    }

    static void configureDefaultLog( ViewHolderTextLog vh ) {
        vh.getLog().setText( "UNHANDLED ITEM RECEIVED" );
        vh.setHighlight( Color.RED );
    }

    private static String getImageUrl( JSONObject image ) {
        String url = null;

        try {
            JSONArray sources = image.getJSONArray( "sources" );
            HashMap<String,String> imageMap = new HashMap<>();

            for ( int j = 0; j < sources.length(); j++ ) {
                JSONObject next = sources.getJSONObject( j );
                String size;
                if ( next.has( "size" ) ) {
                    size = next.getString( "size" ).toUpperCase();
                } else {
                    size = "DEFAULT";
                }
                imageMap.put( size, next.getString( "url" ) );
            }

            if ( imageMap.containsKey( "DEFAULT" ) ) {
                url = imageMap.get( "DEFAULT" );
            } else if ( imageMap.containsKey( "X-LARGE" ) ) {
                url = imageMap.get( "X-LARGE" );
            } else if ( imageMap.containsKey( "LARGE" ) ) {
                url = imageMap.get( "LARGE" );
            } else if ( imageMap.containsKey( "MEDIUM" ) ) {
                url = imageMap.get( "MEDIUM" );
            } else if ( imageMap.containsKey( "SMALL" ) ) {
                url = imageMap.get( "SMALL" );
            } else if ( imageMap.containsKey( "X-SMALL" ) ) {
                url = imageMap.get( "X-SMALL" );
            }
        } catch ( JSONException e ) { Log.e( sTag, e.getMessage() ); }
        return url;
    }
}
