package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Reddit;

public class RedditSteps {

	Reddit reddit = new Reddit(WebDriverFactory.get());
	
	@When("I login reddit with '(.*)' and '(.*)'")
	public void login(String username,String password){
		reddit.enterEmail(username);
		reddit.enterPassword(password);
		reddit.submit();
		
	}

	@Then("I should be logged in reddit")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",reddit.checkLogin());
	}
	

	@When("I logout reddit")
	public void pressLogout(){
		reddit.logout();
	}
}
